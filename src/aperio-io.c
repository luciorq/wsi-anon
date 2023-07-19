#include "aperio-io.h"

// checks if file is aperio
int32_t is_aperio(const char *filename) {
    int32_t result = 0;
    const char *ext = get_filename_ext(filename);

    // check for valid file extension
    if (strcmp(ext, SVS) != 0 && strcmp(ext, TIF) != 0) {
        return result;
    }

    file_t *fp;
    fp = file_open(filename, "rb+");

    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open tiff file.\n");
        return result;
    }

    bool big_tiff = false;
    bool big_endian = false;
    result = check_file_header(fp, &big_endian, &big_tiff);

    if (result != 0) {
        return result;
    }

    struct tiff_file *file;
    file = read_tiff_file(fp, big_tiff, false, big_endian);

    if (file == NULL) {
        fprintf(stderr, "Error: Could not read tiff file.\n");
        file_close(fp);
        return result;
    }

    result = tag_value_contains(fp, file, TIFFTAG_IMAGEDESCRIPTION, "Aperio");

    if (result == -1) {
        fprintf(stderr, "Error: Could not find aperio label directory.\n");
    }

    // is aperio
    file_close(fp);
    return (result == 1);
}

// searches for tags in image description Data and replaces its values with equal amount of X's
char *override_image_description(char *result, char *delimiter, const char *pseudonym) {
    const char *value = get_string_between_delimiters(result, delimiter, "|");
    // check if tag is not an empty string
    if (value[0] != '\0') {
        char *replacement = create_replacement_string(*pseudonym, strlen(value));
        result = replace_str(result, value, replacement);
    }
    return result;
}

/*
struct metadata {
    const char *key;
    const char *value;
};

// TODO: members are incomplete --> view concept again or comment in members below!!
struct all_metadata {
    struct metadata **metadata;
    //size_t length;
};

// TODO: members are incomplete --> view concept again or comment in members below!!
struct wsi_data {
    int8_t format;
    const char *filename;
    //struct associated_image_data **label;
    //struct associated_image_data **macro;
    struct all_metadata **metadata;
};


STUDENT* students = malloc(numStudents * sizeof *students);
    for (x = 0; x < numStudents; x++){
        students[x].firstName=(char*)malloc(sizeof(char*));
        scanf("%s",students[x].firstName);
        students[x].lastName=(char*)malloc(sizeof(char*));
        scanf("%s",students[x].lastName);
        scanf("%d",&students[x].day);
        scanf("%d",&students[x].month);
        scanf("%d",&students[x].year);
    }


*/

struct metadata *get_metadata_in_aperio() {
    // TODO: open and read file here
    // TODO: replace 2 with actual number of metadata that was found in the file
    // TODO: check if strndup isn't more efficient
    // TODO: iterate through all metadata
    // TODO: get correct value
    // TODO: handle pointer warnings
    // TODO: run til all metadata was found
    int8_t num_of_entries = 3;
    struct metadata *metadata = malloc(sizeof(struct metadata));
    struct metadata_attribute **metadata_attributes = malloc(num_of_entries * sizeof(struct metadata_attribute));
    for (int8_t metadata_id = 0; metadata_id < num_of_entries; metadata_id++) {
        struct metadata_attribute *single_attribute = malloc(sizeof(metadata));
        single_attribute->key = APERIO_FILENAME_TAG;
        single_attribute->value = concat_str("TEST ", int32_to_str(metadata_id));
        metadata_attributes[metadata_id] = single_attribute;
    }
    metadata->attributes = metadata_attributes;
    metadata->length = num_of_entries;
    return metadata;
}

// removes all metadata
int32_t remove_metadata_in_aperio(file_t *fp, struct tiff_file *file, const char *pseudonym) {
    for (uint32_t i = 0; i < file->used; i++) {
        struct tiff_directory dir = file->directories[i];
        for (uint32_t j = 0; j < dir.count; j++) {
            struct tiff_entry entry = dir.entries[j];
            if (entry.tag == TIFFTAG_IMAGEDESCRIPTION) {
                // get requested image tag from file
                file_seek(fp, entry.offset, SEEK_SET);
                int32_t entry_size = get_size_of_value(entry.type, &entry.count);

                char buffer[entry_size * entry.count];
                if (file_read(&buffer, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not read tag image description.\n");
                    return -1;
                }

                bool rewrite = false;
                char *result = buffer;

                if (contains(result, APERIO_FILENAME_TAG)) {
                    result = override_image_description(result, APERIO_FILENAME_TAG, pseudonym);
                    rewrite = true;
                }

                if (contains(result, APERIO_USER_TAG)) {
                    result = override_image_description(result, APERIO_USER_TAG, pseudonym);
                    rewrite = true;
                }

                if (contains(result, APERIO_DATE_TAG)) {
                    result = override_image_description(result, APERIO_DATE_TAG, pseudonym);
                    rewrite = true;
                }

                if (contains(result, APERIO_BARCODE_TAG)) {
                    result = override_image_description(result, APERIO_BARCODE_TAG, pseudonym);
                    rewrite = true;
                }

                if (rewrite == true) {
                    file_seek(fp, entry.offset, SEEK_SET);
                    if (file_write(result, entry.count, entry_size, fp) != 1) {
                        fprintf(stderr, "Error: Could not overwrite image description.\n");
                        return -1;
                    }
                }
            }
        }
    }
    return 1;
}

// macro image for gt450 needs to be treated differently because it is JPEG encoded.
// therefore we need to convert it to LZW compression
int32_t change_macro_image_compression_gt450(file_t *fp, struct tiff_file *file, int32_t directory) {
    struct tiff_directory dir = file->directories[directory];
    for (uint32_t i = 0; i < dir.count; i++) {
        struct tiff_entry entry = dir.entries[i];
        if (entry.tag == TIFFTAG_COMPRESSION) {
            if (file_seek(fp, entry.start + 12, SEEK_SET)) {
                fprintf(stderr, "Error: Failed to seek to offset %" PRIu64 ".\n", entry.offset);
                continue;
            }
            uint64_t lzw_com = COMPRESSION_LZW;
            if (!file_write(&lzw_com, 1, sizeof(uint64_t), fp)) {
                fprintf(stderr, "Error: Wiping image data failed.\n");
                return -1;
            }
            break;
        }
    }
    return 0;
}

// anonymizes aperio file
int32_t handle_aperio(const char **filename, const char *new_filename, const char *pseudonym_metadata,
                      struct anon_configuration configuration) {

    fprintf(stdout, "Anonymizing Aperio WSI...\n");

    const char *ext = get_filename_ext(*filename);

    bool is_svs = strcmp(ext, SVS) == 0;

    if (!configuration.do_inplace) {
        // check if filename is svs or tif here
        *filename = duplicate_file(*filename, new_filename, is_svs ? DOT_SVS : DOT_TIF);
    }

    file_t *fp;
    fp = file_open(*filename, "rb+");

    bool big_tiff = false;
    bool big_endian = false;
    int32_t result = check_file_header(fp, &big_endian, &big_tiff);

    if (result != 0) {
        file_close(fp);
        return result;
    }

    struct tiff_file *file;
    file = read_tiff_file(fp, big_tiff, false, big_endian);

    if (file == NULL) {
        fprintf(stderr, "Error: Could not read tiff file.\n");
        return -1;
    }

    int32_t _is_aperio_gt450 = tag_value_contains(fp, file, TIFFTAG_IMAGEDESCRIPTION, "GT450");

    // delete label image
    int32_t label_dir = 0;
    if (_is_aperio_gt450 == 1) {
        label_dir = get_aperio_gt450_dir_by_name(file, LABEL);
    } else {
        label_dir = get_directory_by_tag_and_value(fp, file, TIFFTAG_IMAGEDESCRIPTION, LABEL);
    }

    if (label_dir == -1) {
        fprintf(stderr, "Error: Could not find IFD of label image.\n");
        return -1;
    }

    struct tiff_directory dir = file->directories[label_dir];
    result = wipe_directory(fp, &dir, false, big_endian, big_tiff, LZW_CLEARCODE, NULL);

    if (result != 0) {
        free_tiff_file(file);
        file_close(fp);
        return result;
    }

    // delete macro image
    int32_t macro_dir = -1;
    if (!configuration.keep_macro_image) {
        if (_is_aperio_gt450 == 1) {
            macro_dir = get_aperio_gt450_dir_by_name(file, MACRO);
        } else {
            macro_dir = get_directory_by_tag_and_value(fp, file, TIFFTAG_IMAGEDESCRIPTION, MACRO);
        }

        if (macro_dir == -1) {
            fprintf(stderr, "Error: Could not find IFD of macro image.\n");
            return -1;
        }

        struct tiff_directory dir = file->directories[macro_dir];
        result = wipe_directory(fp, &dir, false, big_endian, big_tiff, NULL, NULL);

        if (_is_aperio_gt450 == 1) {
            result = change_macro_image_compression_gt450(fp, file, macro_dir);
        }

        if (result != 0) {
            free_tiff_file(file);
            file_close(fp);
            return result;
        }
    }

    remove_metadata_in_aperio(fp, file, pseudonym_metadata);

    if (!configuration.disable_unlinking) {
        unlink_directory(fp, file, label_dir, false);
        unlink_directory(fp, file, macro_dir, false);
    }

    // clean up
    free_tiff_file(file);
    file_close(fp);
    return result;
}