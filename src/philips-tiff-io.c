#include "philips-tiff-io.h"

// checks if file is Philips TIFF
int32_t is_philips_tiff(const char *filename) {
    int32_t result = 0;
    const char *ext = get_filename_ext(filename);

    // check for file extension
    if (strcmp(ext, TIFF) != 0) {
        return 0;
    }

    file_t *fp;
    fp = file_open(filename, "r+");

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

    // check if the Software tag starts with Philips
    result = tag_value_contains(fp, file, TIFFTAG_SOFTWARE, "Philips");

    if (result == -1) {
        fprintf(stderr, "Error: Could not find value in Software tag.\n");
    }

    // is Philips' TIFF
    file_close(fp);
    return (result == 1);
}

// remove label image and macro image
int32_t wipe_philips_image_data(file_t *fp, struct tiff_file *file, char *image_type) {
    for (uint64_t i = 0; i < file->used; i++) {
        struct tiff_directory dir = file->directories[i];
        for (uint64_t j = 0; j < dir.count; j++) {
            struct tiff_entry entry = dir.entries[j];
            if (entry.tag == TIFFTAG_IMAGEDESCRIPTION) {
                // get requested image tag from file
                file_seek(fp, entry.offset, SEEK_SET);
                int32_t entry_size = get_size_of_value(entry.type, &entry.count);

                char *buffer = malloc(sizeof(char) * entry_size * entry.count);
                if (file_read(buffer, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not read tag image description.\n");
                    return -1;
                }

                char *result = buffer;
                bool rewrite = false;

                // check for label and macro image in image description
                if (contains(result, image_type)) {

                    // get image data string
                    const char *image_data = get_string_between_delimiters(result, PHILIPS_OBJECT, image_type);
                    image_data = get_string_between_delimiters(image_data, PHILIPS_IMAGE_DATA, PHILIPS_ATT_OPEN);
                    image_data = get_string_between_delimiters(
                        image_data, concat_str(PHILIPS_DELIMITER_STR, PHILIPS_CLOSING_SYMBOL), PHILIPS_ATT_END);

                    // set height and width to 1
                    int32_t height = 1;
                    int32_t width = 1;

                    // alloc with height and width and fill with 255 for a white image
                    unsigned char *white_image = (unsigned char *)malloc((height * width) * sizeof(unsigned char));
                    memset(white_image, 255, height * width);

                    // create white jpg image
                    jpec_enc_t *e = jpec_enc_new(white_image, width, height);
                    int32_t len;
                    const uint8_t *jpeg = jpec_enc_run(e, &len);

                    // encode new image data and check if string is longer than original string,
                    // replace old base64-encoded string afterwards
                    char *new_image_data = b64_encode(jpeg, strlen(image_data));
                    if (strlen(new_image_data) > strlen(image_data)) {
                        new_image_data[strlen(image_data)] = '\0';
                    }

                    result = replace_str(result, image_data, new_image_data);
                    rewrite = true;

                    // free memory and release encoder
                    free(white_image);
                    free(new_image_data);
                    jpec_enc_del(e);
                }

                // alter image in image description
                if (rewrite) {
                    strcpy(buffer, result);
                    file_seek(fp, entry.offset, SEEK_SET);
                    if (!file_write(buffer, entry.count, entry_size, fp)) {
                        fprintf(stderr, "Error: changing image description failed.\n");
                        free(buffer);
                        return -1;
                    }
                    free(buffer);
                }
            }
        }
    }
    return 0;
}

// anonymizes metadata from Philips' TIFF file
int32_t anonymize_philips_metadata(file_t *fp, struct tiff_file *file) {
    for (uint64_t i = 0; i < file->used; i++) {
        struct tiff_directory dir = file->directories[i];
        for (uint64_t j = 0; j < dir.count; j++) {
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

                // Datetime attribute is substituted with minimum possible value
                if (contains(result, PHILIPS_DATETIME_ATT)) {
                    const char *value = get_value_from_attribute(result, PHILIPS_DATETIME_ATT);
                    result = replace_str(result, value, PHILIPS_MIN_DATETIME);
                    rewrite = true;
                }

                // replaced with arbitrary value
                if (contains(result, PHILIPS_SERIAL_ATT)) {
                    result = anonymize_value_of_attribute(result, PHILIPS_SERIAL_ATT);
                    rewrite = true;
                }

                // wipes complete section of given attribute
                if (contains(result, PHILIPS_SLOT_ATT)) {
                    result = wipe_section_of_attribute(result, PHILIPS_SLOT_ATT);
                    rewrite = true;
                }

                // wipes complete section of given attribute
                if (contains(result, PHILIPS_RACK_ATT)) {
                    result = wipe_section_of_attribute(result, PHILIPS_RACK_ATT);
                    rewrite = true;
                }

                // replace with arbitrary value
                if (contains(result, PHILIPS_OPERID_ATT)) {
                    result = anonymize_value_of_attribute(result, PHILIPS_OPERID_ATT);
                    rewrite = true;
                }

                // replace with arbitrary value
                if (contains(result, PHILIPS_BARCODE_ATT)) {
                    result = anonymize_value_of_attribute(result, PHILIPS_BARCODE_ATT);
                    rewrite = true;
                }

                // replace with arbitrary value
                if (contains(result, PHILIPS_SOURCE_FILE_ATT)) {
                    result = anonymize_value_of_attribute(result, PHILIPS_SOURCE_FILE_ATT);
                    rewrite = true;
                }

                // alters image description
                if (rewrite) {
                    strcpy(buffer, result);
                    file_seek(fp, entry.offset, SEEK_SET);
                    if (!file_write(buffer, entry.count, entry_size, fp)) {
                        fprintf(stderr, "Error: changing Image Description failed.\n");
                        return -1;
                    }
                }
                return 1;
            }
        }
    }
    return 1;
}

// anonymize Philips' TIFF
int32_t handle_philips_tiff(const char **filename, const char *new_label_name, bool keep_macro_image,
                            bool disable_unlinking, bool do_inplace) {

    if (disable_unlinking) {
        fprintf(stderr, "Error: Cannot disable unlinking in Philips' TIFF file.\n");
    }

    fprintf(stdout, "Anonymize Philips' TIFF WSI...\n");

    if (!do_inplace) {
        *filename = duplicate_file(*filename, new_label_name, DOT_TIFF);
    }

    file_t *fp;
    fp = file_open(*filename, "r+");

    // philips tiff files can be stored as single-tiff TIFF or BigTIFF format
    bool big_tiff = false;
    bool big_endian = false;
    int32_t result = check_file_header(fp, &big_endian, &big_tiff);

    if (result != 0) {
        file_close(fp);
        return result;
    }

    struct tiff_file *file;
    file = read_tiff_file(fp, big_tiff, false, big_endian);

    // if file could not be opened
    if (fp == NULL) {
        return -1;
    }

    // remove label image || TODO: check why removal is not working
    result = wipe_philips_image_data(fp, file, PHILIPS_LABELIMAGE);

    if (result == -1) {
        fprintf(stderr, "Error: Could not wipe label image from file.\n");
    }

    // remove macro image || TODO: check why removal is not working
    if (!keep_macro_image) {
        result = wipe_philips_image_data(fp, file, PHILIPS_MACROIMAGE);

        if (result == -1) {
            fprintf(stderr, "Error: Could not wipe macro image from file.\n");
        }
    }

    anonymize_philips_metadata(fp, file);

    // clean up
    file_close(fp);
    return result;
}
