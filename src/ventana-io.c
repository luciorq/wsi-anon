#include "ventana-io.h"

// checks if file is in ventana format
int32_t is_ventana(const char *filename) {
    int32_t result = 0;
    const char *ext = get_filename_ext(filename);

    bool is_bif = strcmp(ext, BIF) == 0;
    bool is_tif = strcmp(ext, TIF) == 0;

    if (!is_bif && !is_tif) {
        return result;
    }

    file_t *fp = file_open(filename, "r+");

    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open tiff file.\n");
        return result;
    }

    bool big_tiff = false;
    bool big_endian = false;

    result = check_file_header(fp, &big_endian, &big_tiff);

    if (!big_tiff || result == -1) {
        fprintf(stderr, "Error: Not a valid Ventana file.\n");
        file_close(fp);
        return -1;
    }

    struct tiff_file *file;
    file = read_tiff_file(fp, big_tiff, false, big_endian);

    if (file == NULL) {
        fprintf(stderr, "Error: Could not read tiff file.\n");
        file_close(fp);
        return result;
    }

    result = tag_value_contains(fp, file, TIFFTAG_XMP, "iScan");

    if (result == -1) {
        fprintf(stderr, "Error: Could not find XMP tag.\n");
        file_close(fp);
        return result;
    }

    // is ventana
    file_close(fp);
    return result;
}

// gets the label directory of ventana file
int64_t get_ventana_label_dir(file_t *fp, struct tiff_file *file) {

    for (uint64_t i = 0; i < file->used; i++) {

        struct tiff_directory dir = file->directories[i];
        for (uint64_t j = 0; j < dir.count; j++) {
            struct tiff_entry entry = dir.entries[j];
            if (entry.tag == TIFFTAG_IMAGEDESCRIPTION) {
                file_seek(fp, entry.offset, SEEK_SET);
                int32_t entry_size = get_size_of_value(entry.type, &entry.count);

                char *buffer = malloc(entry_size * entry.count);
                if (file_read(buffer, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not read image description.\n");
                    free(buffer);
                    return -1;
                }

                if (contains(buffer, "Label")) {
                    free(buffer);
                    return i;
                }
                free(buffer);
            }
        }
    }
    return -1;
}

// wipes the label directory of ventana file by replacing bytes with zeros
int32_t wipe_label_ventana(file_t *fp, struct tiff_directory *dir) {
    int32_t tile_offsets = -1;
    int32_t tile_byte_counts = -1;

    for (uint64_t i = 0; i < dir->count; i++) {
        struct tiff_entry entry = dir->entries[i];

        if (entry.tag == TIFFTAG_TILEOFFSETS) {
            tile_offsets = entry.offset;
        } else if (entry.tag == TIFFTAG_TILEBYTECOUNTS) {
            tile_byte_counts = entry.count;
        }
    }

    if (tile_offsets == -1 || tile_byte_counts == -1) {
        fprintf(stderr, "Error: Could not retrieve tile offsets or tile byte counts.\n");
        return -1;
    }

    file_seek(fp, tile_offsets, SEEK_SET);

    // fill strip with zeros
    char *strip = create_pre_suffixed_char_array('0', tile_byte_counts, NULL, NULL);
    if (!file_write(strip, 1, tile_byte_counts, fp)) {
        fprintf(stderr, "Error: Wiping image data failed.\n");
        free(strip);
        return -1;
    }
    free(strip);

    return 0;
}

// wipes and unlinks directory
int32_t wipe_and_unlink_ventana_directory(file_t *fp, struct tiff_file *file, int64_t directory,
                                          bool disable_unlinking) {
    // surpress compiler warning; remove when unlinking is implemented
    UNUSED(disable_unlinking);

    struct tiff_directory dir = file->directories[directory];

    int32_t result = wipe_label_ventana(fp, &dir);

    // ToDo: check if unlinking for overview image (IFD 0) for .bif files is possible
    /*
    if (result != -1 && !disable_unlinking) {
        result = unlink_directory(fp, file, directory, false);
    }
    */

    return result;
}

// searches for attributes in XMP Data and replaces its values with equal amount of empty spaces
char *wipe_xmp_data(char *result, char *delimiter1, char *delimiter2) {
    const char *value = get_string_between_delimiters(result, delimiter1, delimiter2);
    const char *sliced_string = slice_str(value, 1, strlen(value) - 2);
    char *replacement = create_replacement_string(' ', strlen(sliced_string));
    return replace_str(result, sliced_string, replacement);
}

// anonymizes metadata in XMP Tags of ventana file
int32_t remove_metadata_in_ventana(file_t *fp, struct tiff_file *file) {
    for (uint64_t i = 0; i < file->used; i++) {

        struct tiff_directory dir = file->directories[i];
        for (uint64_t j = 0; j < dir.count; j++) {

            struct tiff_entry entry = dir.entries[j];

            // searches for XMP Tag in all directories and removes metadata in it
            if (entry.tag == TIFFTAG_XMP) {
                file_seek(fp, entry.offset, SEEK_SET);
                int32_t entry_size = get_size_of_value(entry.type, &entry.count);
                char *buffer = malloc(entry_size * entry.count);
                if (file_read(buffer, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not read XMP Tag.\n");
                    free(buffer);
                    return -1;
                }

                char *result = buffer;
                bool rewrite = false;

                if (contains(result, VENTANA_BASENAME_ATT)) {
                    result = wipe_xmp_data(result, VENTANA_BASENAME_ATT, " ");
                    rewrite = true;
                }

                if (contains(result, VENTANA_FILENAME_ATT)) {
                    result = wipe_xmp_data(result, VENTANA_FILENAME_ATT, " ");
                    rewrite = true;
                }

                if (contains(result, VENTANA_UNITNUMBER_ATT)) {
                    result = wipe_xmp_data(result, VENTANA_UNITNUMBER_ATT, " ");
                    rewrite = true;
                }

                if (contains(result, VENTANA_USERNAME_ATT)) {
                    result = wipe_xmp_data(result, VENTANA_USERNAME_ATT, " ");
                    rewrite = true;
                }

                // checks if attribute occurs more than once in directory
                if (contains(result, VENTANA_BUILDDATE1_ATT)) {
                    int32_t count = count_contains(result, VENTANA_BUILDDATE1_ATT);
                    for (int32_t i = 0; i <= count; i++) {
                        const char *value = get_string_between_delimiters(result, VENTANA_BUILDDATE1_ATT, "\'");
                        char *replacement = create_replacement_string(' ', strlen(value));
                        result = replace_str(result, value, replacement);
                    }
                    rewrite = true;
                }

                if (contains(result, VENTANA_BUILDDATE2_ATT)) {
                    const char *value = get_string_between_delimiters(result, VENTANA_BUILDDATE2_ATT, "\"");
                    char *replacement = create_replacement_string(' ', strlen(value));
                    result = replace_str(result, value, replacement);
                    rewrite = true;
                }

                if (contains(result, VENTANA_BARCODE1D_ATT)) {
                    result = wipe_xmp_data(result, VENTANA_BARCODE1D_ATT, " ");
                    rewrite = true;
                }

                if (contains(result, VENTANA_BARCODE2D_ATT)) {
                    result = wipe_xmp_data(result, VENTANA_BARCODE2D_ATT, " ");
                    rewrite = true;
                }

                // alters XML data of XMP tag
                if (rewrite) {
                    file_seek(fp, entry.offset, SEEK_SET);
                    if (!file_write(result, entry_size, entry.count, fp)) {
                        fprintf(stderr, "Error: changing XML Data in XMP Tag failed.\n");
                        free(buffer);
                        return -1;
                    }
                }
                free(buffer);
            }

            // remove value in DATE_TIME tag
            if (entry.tag == TIFFTAG_DATETIME) {
                file_seek(fp, entry.offset, SEEK_SET);
                int32_t entry_size = get_size_of_value(entry.type, &entry.count);
                char *buffer = malloc(entry_size * entry.count);
                if (file_read(buffer, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not read DATE_TIME Tag.\n");
                    free(buffer);
                    return -1;
                }
                char *replacement = create_replacement_string(' ', strlen(buffer));
                buffer = replace_str(buffer, buffer, replacement);
                file_seek(fp, entry.offset, SEEK_SET);
                if (!file_write(buffer, entry_size, entry.count, fp)) {
                    fprintf(stderr, "Error: changing data in DATE_TIME Tag failed.\n");
                    free(buffer);
                    return -1;
                }
                free(buffer);
            }
        }
    }
    return 1;
}

// anonymizes ventana file
int32_t handle_ventana(const char **filename, const char *new_label_name, bool keep_macro_image, bool disable_unlinking,
                       bool do_inplace) {

    if (keep_macro_image) {
        fprintf(stderr, "Error: Cannot keep macro image in Ventana file.\n");
    }

    fprintf(stdout, "Anonymize Ventana WSI...\n");

    const char *ext = get_filename_ext(*filename);

    bool is_bif = strcmp(ext, BIF) == 0;

    if (!do_inplace) {
        *filename = duplicate_file(*filename, new_label_name, is_bif ? DOT_BIF : DOT_TIF);
    }

    file_t *fp = file_open(*filename, "r+");

    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open tiff file.\n");
        return -1;
    }

    bool big_tiff = false;
    bool big_endian = false;
    int32_t result = check_file_header(fp, &big_endian, &big_tiff);

    if (result == -1) {
        file_close(fp);
        return -1;
    }

    struct tiff_file *file;
    file = read_tiff_file(fp, big_tiff, false, big_endian);

    if (file == NULL) {
        fprintf(stderr, "Error: Could not read tiff file.\n");
        file_close(fp);
        return -1;
    }

    int64_t label_dir = get_ventana_label_dir(fp, file);

    if (label_dir == -1) {
        fprintf(stderr, "Error: Could not find Image File Directory of Label image.\n");
        return -1;
    }

    result = wipe_and_unlink_ventana_directory(fp, file, label_dir, disable_unlinking);

    if (result == -1) {
        free_tiff_file(file);
        file_close(fp);
        return -1;
    }

    remove_metadata_in_ventana(fp, file);

    // clean up
    free_tiff_file(file);
    file_close(fp);
    return result;
}