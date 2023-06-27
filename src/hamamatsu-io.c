#include "hamamatsu-io.h"

// checks if the file is hamamatsu
int32_t is_hamamatsu(const char *filename) {
    int32_t result = 0;
    const char *ext = get_filename_ext(filename);

    // check for valid file extension
    if (strcmp(ext, NDPI) != 0) {
        return result;
    }

    // open file
    file_t *fp = file_open(filename, "r+");
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open tiff file.\n");
        return result;
    }

    // check if ndpi tiff tags are present
    bool big_tiff = false;
    bool big_endian = false;
    result = check_file_header(fp, &big_endian, &big_tiff);

    file_close(fp);
    return (result == 0);
}

// retrieve the label directory from the tiff file structure
int32_t get_hamamatsu_macro_dir(struct tiff_file *file, file_t *fp, bool big_endian) {
    for (uint64_t i = 0; i < file->used; i++) {
        struct tiff_directory temp_dir = file->directories[i];

        for (uint64_t j = 0; j < temp_dir.count; j++) {
            struct tiff_entry temp_entry = temp_dir.entries[j];

            if (temp_entry.tag == NDPI_SOURCELENS) {
                int32_t entry_size = get_size_of_value(temp_entry.type, &temp_entry.count);

                if (entry_size) {

                    if (entry_size && temp_entry.type == FLOAT) {
                        float *v_buffer = (float *)malloc(entry_size * temp_entry.count);

                        // we need to step 8 bytes from start pointer
                        // to get the expected value
                        uint64_t new_start = temp_entry.start + 8;
                        if (file_seek(fp, new_start, SEEK_SET)) {
                            fprintf(stderr, "Error: Failed to seek to offset %" PRIu64 ".\n", new_start);
                            return -1;
                        }
                        if (file_read(v_buffer, entry_size, temp_entry.count, fp) != 1) {
                            fprintf(stderr, "Error: Failed to read entry value.\n");
                            return -1;
                        }
                        fix_byte_order(v_buffer, sizeof(float), 1, big_endian);

                        if (*v_buffer == -1) {
                            // we found the label directory
                            return i;
                        }
                    }
                }
            }
        }
    }
    return -1;
}

// removes all metadata
int32_t remove_metadata_in_hamamatsu(file_t *fp, struct tiff_file *file) {
    for (uint32_t i = 0; i < file->used; i++) {
        struct tiff_directory dir = file->directories[i];
        for (uint32_t j = 0; j < dir.count; j++) {
            struct tiff_entry entry = dir.entries[j];
            if (entry.tag == TIFFTAG_DATETIME) {
                file_seek(fp, entry.offset, SEEK_SET);
                int32_t entry_size = get_size_of_value(entry.type, &entry.count);

                char buffer[entry_size * entry.count];
                if (file_read(&buffer, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not read tag DateTime.\n");
                    return -1;
                }

                char *replacement = create_replacement_string('X', strlen(buffer));

                file_seek(fp, entry.offset, SEEK_SET);

                if (file_write(replacement, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not overwrite tag DateTime.\n");
                    return -1;
                }
            }
        }
    }
    return 1;
}

// anonymizes hamamatsu file
int32_t handle_hamamatsu(const char **filename, const char *new_label_name, bool keep_macro_image,
                         bool disable_unlinking, bool do_inplace) {

    if (keep_macro_image) {
        fprintf(stderr, "Error: Macro image will be wiped if found.\n");
    }

    fprintf(stdout, "Anonymize Hamamatsu WSI...\n");
    if (!do_inplace) {
        *filename = duplicate_file(*filename, new_label_name, DOT_NDPI);
    }

    file_t *fp;
    fp = file_open(*filename, "r+");

    bool big_tiff = false;
    bool big_endian = false;
    // we check the header again, so the pointer
    // will be placed at the expected position
    int32_t result = check_file_header(fp, &big_endian, &big_tiff);
    if (result != 0) {
        fprintf(stderr, "Error: Could not read header file.\n");
        file_close(fp);
        return result;
    }

    // read the file structure
    struct tiff_file *file;
    file = read_tiff_file(fp, big_tiff, true, big_endian);
    if (file == NULL) {
        fprintf(stderr, "Error: Could not read tiff file.\n");
        file_close(fp);
        return -1;
    }

    // find the macro directory
    int32_t dir_count = get_hamamatsu_macro_dir(file, fp, big_endian);
    if (dir_count == -1) {
        fprintf(stderr, "Error: No macro directory.\n");
        free_tiff_file(file);
        file_close(fp);
        return -1;
    }

    struct tiff_directory dir = file->directories[dir_count];

    // wipe macro data from directory
    // check for JPEG SOI header in macro dir
    result = wipe_directory(fp, &dir, true, big_endian, big_tiff, JPEG_SOI, NULL);

    if (result != 0) {
        free_tiff_file(file);
        file_close(fp);
        return result;
    }

    if (!disable_unlinking) {
        // unlink the empty macro directory from file structure
        result = unlink_directory(fp, file, dir_count, true);
    }

    remove_metadata_in_hamamatsu(fp, file);

    free_tiff_file(file);
    file_close(fp);

    return result;
}