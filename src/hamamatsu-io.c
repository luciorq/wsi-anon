// checks if the file is hamamatsu
int32_t is_format(const char *filename) {
    int32_t result = 0;
    const char *ext = get_filename_ext(filename);

    // check for valid file extension
    if (strcmp(ext, NDPI) != 0) {
        return result;
    }

    // check if ndpi tiff tags are present
    file_t *fp = file_open(filename, "r+");
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

                    if (temp_entry.type == FLOAT) {
                        float *v_buffer = (float *)malloc(entry_size * temp_entry.count);

                        // we need to step 8 bytes from start pointer
                        // to get the expected value
                        uint64_t new_start = temp_entry.start + 8;
                        if (file_seek(fp, new_start, SEEK_SET)) {
                            fprintf(stderr, "Error: Failed to seek to offset %lu.\n", new_start);
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

// anonymizes hamamatsu file
int32_t handle_format(const char **filename, const char *new_label_name, bool disable_unlinking,
                      bool do_inplace) {
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
    result = wipe_directory(fp, &dir, true, big_endian, JPEG_SOI, NULL);

    if (result != 0) {
        free_tiff_file(file);
        file_close(fp);
        return result;
    }

    if (!disable_unlinking) {
        // unlink the empty macro directory from file structure
        result = unlink_directory(fp, file, dir_count, true);
    }

    free_tiff_file(file);
    file_close(fp);

    return result;
}