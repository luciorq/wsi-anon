#include "hamamatsu-io.h"

// checks if the file is hamamatsu
int32_t is_hamamatsu(const char *filename) {
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

int32_t wipe_directory(file_t *fp, struct tiff_directory *dir, bool ndpi, bool big_endian,
                       const char *prefix, const char *suffix) {
    int32_t size_offsets;
    int32_t size_lengths;
    // gather strip offsets and lengths form tiff directory
    uint32_t *strip_offsets =
        read_pointer_by_tag(fp, dir, TIFFTAG_STRIPOFFSETS, ndpi, big_endian, &size_offsets);
    uint32_t *strip_lengths =
        read_pointer_by_tag(fp, dir, TIFFTAG_STRIPBYTECOUNTS, ndpi, big_endian, &size_lengths);

    if (strip_offsets == NULL || strip_lengths == NULL) {
        fprintf(stderr, "Error: Could not retrieve strip offset and length.\n");
        return -1;
    }

    if (size_offsets != size_lengths) {
        fprintf(stderr, "Error: Length of strip offsets and lengths are not matching.\n");
        return -1;
    }

    for (int32_t i = 0; i < size_offsets; i++) {
        file_seek(fp, strip_offsets[i], SEEK_SET);

        if (prefix != NULL) {
            // we check the head of the directory offset for a given
            // prefix. if the head is not equal to the given prefix
            // we do not wipe the label data
            size_t prefix_len = strlen(prefix);
            char *buf = (char *)malloc(prefix_len + 1);
            buf[prefix_len] = '\0';

            if (file_read(buf, prefix_len, 1, fp) != 1) {
                fprintf(stderr, "Error: Could not read strip prefix.\n");
                free(buf);
                return -1;
            }

            if (strcmp(prefix, buf) != 0) {
                fprintf(stderr, "Error: Prefix in data strip not found.\n");
                free(buf);
                return -1;
            }

            file_seek(fp, strip_offsets[i], SEEK_SET);
            free(buf);
        }

        // fill strip with zeros
        char *strip = get_empty_char_buffer("0", strip_lengths[i], prefix, suffix);
        if (!file_write(strip, 1, strip_lengths[i], fp)) {
            fprintf(stderr, "Error: Wiping image data failed.\n");
            free(strip);
            return -1;
        }
        free(strip);
    }

    return 0;
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