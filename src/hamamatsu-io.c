#include "hamamatsu-io.h"

// TODO: delete this (obsolete)
// checks if the file is hamamatsu
int32_t is_hamamatsu(const char *filename) {
    int32_t result = 0;
    const char *ext = get_filename_ext(filename);

    // check for valid file extension
    if (strcmp(ext, NDPI) != 0) {
        return result;
    }

    // check if ndpi tiff tags are present
    file_t *fp = file_open(filename, "rb+");
    bool big_tiff = false;
    bool big_endian = false;
    result = check_file_header(fp, &big_endian, &big_tiff);

    file_close(fp);
    return (result == 0);
}

// retrieve the macro directory in order to wipe label image from the tiff file structure
int32_t get_hamamatsu_macro_dir(struct tiff_file *file, file_t *fp, bool big_endian) {
    for (uint64_t i = 0; i < file->used; i++) {
        struct tiff_directory temp_dir = file->directories[i];

        for (uint64_t j = 0; j < temp_dir.count; j++) {
            struct tiff_entry temp_entry = temp_dir.entries[j];

            if (temp_entry.tag == NDPI_SOURCELENS) {
                int32_t entry_size = get_size_of_value(temp_entry.type, &temp_entry.count);

                if (entry_size && temp_entry.type == TIFF_FLOAT) {
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

                    // SourceLens equals -1 if macro directory containing the label image was found
                    if (*v_buffer == -1) {
                        return i;
                    }
                }
            }
        }
    }
    return -1;
}

struct metadata *get_metadata_hamamatsu(file_t *fp, struct tiff_file *file) {
    // initialize metadata_attribute struct
    // TODO: find better value to determine size for malloc of metadata
    struct metadata_attribute **attributes = malloc(sizeof(**attributes) * 15);
    int8_t metadata_id = 0;

    // iterate over directories in tiff file
    for (uint32_t i = 0; i < file->used; i++) {
        struct tiff_directory dir = file->directories[i];
        for (uint32_t j = 0; j < dir.count; j++) {
            struct tiff_entry entry = dir.entries[j];

            // entry with Datetime tag contains metadata
            if (entry.tag == TIFFTAG_DATETIME) {
                file_seek(fp, entry.offset, SEEK_SET);
                int32_t entry_size = get_size_of_value(entry.type, &entry.count);

                // read content of Datetime into buffer
                char buffer[entry_size * entry.count];
                if (file_read(&buffer, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not read tag DateTime.\n");
                    return NULL;
                }

                // add metadata
                struct metadata_attribute *single_attribute = malloc(sizeof(*single_attribute));
                single_attribute->key = "Datetime";
                single_attribute->value = strdup(buffer);
                if (single_attribute != NULL) {
                    attributes[metadata_id++] = single_attribute;
                }
            }
        }
    }

    // add all found metadata
    struct metadata *metadata_attributes = malloc(sizeof(*metadata_attributes));
    metadata_attributes->attributes = attributes;
    metadata_attributes->length = metadata_id;
    return metadata_attributes;
}

struct wsi_data *get_wsi_data_hamamatsu(const char *filename) {
    // gets file extension
    int32_t result = 0;
    const char *ext = get_filename_ext(filename);

    // check for valid file extension
    if (strcmp(ext, NDPI) != 0) {
        return NULL;
    }

    // opens file
    file_t *fp = file_open(filename, "rb+");

    // checks if file was successfully opened
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open Hamamatsu file.\n");
        return NULL;
    }

    // checks file details
    bool big_tiff = false;
    bool big_endian = false;
    result = check_file_header(fp, &big_endian, &big_tiff);

    // checks result
    if (result != 0) {
        return NULL;
    }

    // creates tiff file structure
    struct tiff_file *file;
    file = read_tiff_file(fp, big_tiff, true, big_endian);

    // checks result
    if (file == NULL) {
        fprintf(stderr, "Error: Could not read tiff file.\n");
        file_close(fp);
        return NULL;
    }

    // gets all metadata
    struct metadata *metadata_attributes = get_metadata_hamamatsu(fp, file);

    // is Hamamatsu
    // TODO: replace format value and handle more efficiently
    struct wsi_data *wsi_data = malloc(sizeof(*wsi_data));
    wsi_data->format = 1;
    wsi_data->filename = filename;
    wsi_data->metadata_attributes = metadata_attributes;

    // cleanup
    file_close(fp);
    return wsi_data;
}

// TODO: make use of get_metadata_hamamatsu function
// removes all metadata
int32_t remove_metadata_in_hamamatsu(file_t *fp, struct tiff_file *file, const char *pseudonym) {
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

                char *replacement = create_replacement_string(*pseudonym, strlen(buffer));

                file_seek(fp, entry.offset, SEEK_SET);

                if (file_write(replacement, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not overwrite metadata in file.\n");
                    return -1;
                }
                return 0;
            }
        }
    }
    return 0;
}

// anonymizes hamamatsu file
int32_t handle_hamamatsu(const char **filename, const char *new_filename, const char *pseudonym_metadata,
                         struct anon_configuration *configuration) {

    if (configuration->keep_macro_image) {
        fprintf(stderr, "Error: Macro image will be wiped if found.\n");
    }

    fprintf(stdout, "Anonymizing Hamamatsu WSI...\n");

    if (!configuration->do_inplace) {
        *filename = duplicate_file(*filename, new_filename, DOT_NDPI);
    }

    file_t *fp;
    fp = file_open(*filename, "rb+");

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

    // remove metadata
    result = remove_metadata_in_hamamatsu(fp, file, pseudonym_metadata);
    if (result != 0) {
        free_tiff_file(file);
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

    if (!configuration->disable_unlinking) {
        // unlink the empty macro directory from file structure
        result = unlink_directory(fp, file, dir_count, true);
    }

    free_tiff_file(file);
    file_close(fp);

    return result;
}