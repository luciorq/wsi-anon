#include "isyntax-io.h"

// find size up to substring
int32_t get_size_to_substring(file_t *fp, char *substring) {

    file_seek(fp, 0, SEEK_END);
    long file_length = file_tell(fp);
    char *buffer = (char *)malloc(file_length);
    file_seek(fp, 0, SEEK_SET);

    if (file_read(buffer, file_length, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read iSyntax file.\n");
        return -1;
    }

    // finds size
    char *ret = strstr(buffer, substring);
    int32_t size = ret - buffer;

    file_seek(fp, 0, SEEK_SET);
    free(buffer);

    return size;
}

// check if file contains specific value
int32_t file_contains_value(file_t *fp, char *value) {

    file_seek(fp, 0, SEEK_END);
    long size = file_tell(fp);
    char *buffer = (char *)malloc(size);
    file_seek(fp, 0, SEEK_SET);

    if (file_read(buffer, size, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read iSyntax file.\n");
        return 0;
    }

    // searches for value
    if (contains(buffer, value)) {
        free(buffer);
        return 1;
    }

    free(buffer);
    return 0;
}

// checks iSyntax file format
int32_t is_isyntax(const char *filename) {

    const char *ext = get_filename_ext(filename);

    // check for file extension
    if (strcmp(ext, ISYNTAX_EXT) != 0) {
        return 0;
    } else {
        int32_t result = 0;
        file_t *fp = file_open(filename, "r");

        // if file could not be opened
        if (fp == NULL) {
            return result;
        }

        // searches for root node in file for verification
        result = file_contains_value(fp, ISYNTAX_ROOTNODE);

        // if root node could not be found
        if (!result) {
            return result;
        }

        // is iSyntax
        file_close(fp);
        return result;
    }
}

// searches for attribute and replaces its value with equal amount of X's
char *anonymize_value_of_attribute(char *buffer, char *attribute) {

    const char *value = get_string_between_delimiters(buffer, attribute, ISYNTAX_ATT_OPEN);
    value = get_string_between_delimiters(
        value, concat_str(ISYNTAX_DELIMITER_STR, ISYNTAX_CLOSING_SYMBOL), ISYNTAX_ATT_END);

    // check for empty String
    if (strcmp(value, "") != 0) {
        char *replace_with = "X";
        char *replacement = anonymize_string(replace_with, strlen(value));
        return replace_str(buffer, value, replacement);
    }

    return buffer;
}

// returns value for an attribute
const char *get_value_from_attribute(char *buffer, char *attribute) {
    const char *value = get_string_between_delimiters(buffer, attribute, ISYNTAX_ATT_OPEN);
    const char *delimiter =
        get_string_between_delimiters(value, ISYNTAX_ATT_PMSVR, ISYNTAX_CLOSING_SYMBOL);

    // check for datatype
    if (strcmp(delimiter, ISYNTAX_DELIMITER_STR) == 0) {
        return get_string_between_delimiters(
            value, concat_str(ISYNTAX_DELIMITER_STR, ISYNTAX_CLOSING_SYMBOL), ISYNTAX_ATT_END);
    } else if (strcmp(delimiter, ISYNTAX_DELIMITER_INT) == 0) {
        return get_string_between_delimiters(
            value, concat_str(ISYNTAX_DELIMITER_INT, ISYNTAX_CLOSING_SYMBOL), ISYNTAX_ATT_END);
    } else {
        fprintf(stderr, "Unable find value for attribute with this datatype");
        return NULL;
    }
}

// replaces section of passed attribute with empty string
char *wipe_section_of_attribute(char *buffer, char *attribute) {
    const char *section = get_string_between_delimiters(
        buffer, attribute, concat_str(ISYNTAX_ATT_END, ISYNTAX_CLOSING_SYMBOL));
    section = concat_str(attribute, section);
    section = concat_str(section, concat_str(ISYNTAX_ATT_END, ISYNTAX_CLOSING_SYMBOL));
    char *replacement = anonymize_string(" ", strlen(section));
    return replace_str(buffer, section, replacement);
}

// anonymizes metadata from iSyntax file
int32_t anonymize_isyntax_metadata(file_t *fp, int32_t header_size) {

    // gets only XML header
    char *buffer = (char *)malloc(header_size);

    file_seek(fp, 0, SEEK_SET);

    if (file_read(buffer, header_size, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read iSyntax file.\n");
        return -1;
    }

    char *result = buffer;
    bool rewrite = false;

    // Datetime attribute is substituted with minimum possible value
    if (contains(result, ISYNTAX_DATETIME_ATT)) {
        const char *value = get_value_from_attribute(result, ISYNTAX_DATETIME_ATT);
        result = replace_str(result, value, ISYNTAX_MIN_DATETIME);
        rewrite = true;
    }

    // replaced with arbitrary value
    if (contains(result, ISYNTAX_SERIAL_ATT)) {
        result = anonymize_value_of_attribute(result, ISYNTAX_SERIAL_ATT);
        rewrite = true;
    }

    // wipes complete section of given attribute
    if (contains(result, ISYNTAX_SLOT_ATT)) {
        result = wipe_section_of_attribute(result, ISYNTAX_SLOT_ATT);
        rewrite = true;
    }

    // wipes complete section of given attribute
    if (contains(result, ISYNTAX_RACK_ATT)) {
        result = wipe_section_of_attribute(result, ISYNTAX_RACK_ATT);
        rewrite = true;
    }

    // replace with arbitrary value
    if (contains(result, ISYNTAX_OPERID_ATT)) {
        result = anonymize_value_of_attribute(result, ISYNTAX_OPERID_ATT);
        rewrite = true;
    }

    // replace with arbitrary value
    if (contains(result, ISYNTAX_BARCODE_ATT)) {
        result = anonymize_value_of_attribute(result, ISYNTAX_BARCODE_ATT);
        rewrite = true;
    }

    // alters iSyntax file
    if (rewrite) {
        file_seek(fp, 0, SEEK_SET);
        if (!file_write(result, header_size, 1, fp)) {
            fprintf(stderr, "Error: changing XML Header failed.\n");
            free(buffer);
            return -1;
        }
    }

    free(buffer);
    return 1;
}

// remove label image and macro image
int32_t wipe_image_data(file_t *fp, int32_t header_size, char *image_type) {

    // gets only the xml header
    char *buffer = (char *)malloc(header_size);

    file_seek(fp, 0, SEEK_SET);

    if (file_read(buffer, header_size, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read iSyntax file.\n");
        return -1;
    }

    char *result = buffer;
    bool rewrite = false;

    if (contains(result, image_type)) {

        // get image data string
        const char *image_data = get_string_between_delimiters(result, image_type, ISYNTAX_OBJECT);
        image_data = get_string_between_delimiters(image_data, ISYNTAX_IMAGE_DATA, ISYNTAX_DATA);
        image_data = get_string_between_delimiters(
            image_data, concat_str(ISYNTAX_DELIMITER_STR, ISYNTAX_CLOSING_SYMBOL), ISYNTAX_ATT_END);

        // decode base64 string for image data
        size_t decode_size[1];
        decode_size[0] = strlen(image_data);
        unsigned char *decoded_data = b64_decode_ex(image_data, *decode_size, &decode_size[0]);

        // offset and length of bytes for width and height
        size_t pos = -1;
        size_t size_bytes_len = 0;

        // check structure for width and height
        for (int i = 0; i < decode_size[0]; i++) {

            // check prefix of possible SOF section
            if (decoded_data[i] == 255         // xff
                && decoded_data[i + 1] == 192  // xc0
                && decoded_data[i + 2] == 0    // x00
                && decoded_data[i + 3] == 17   // x11
                && decoded_data[i + 4] == 8) { // x08

                // prefix length
                pos = i + 5;

                // check suffix
                for (int j = pos; j < decode_size[0]; j++) {
                    if (decoded_data[j] == 3 &&     // x03
                        decoded_data[j + 1] == 1) { // x01

                        bool is_suffix = false;

                        for (int z = j + 2; z < decode_size[0]; z++) {

                            // if another /x03/x01 was found, it is not SOF section
                            if (decoded_data[z] == 3 &&     // x03
                                decoded_data[z + 1] == 1) { // x01
                                break;
                            }

                            // if next section (huffman table) was found, then it is SOF section
                            if (decoded_data[z] == 255 &&     // xff
                                decoded_data[z + 1] == 196) { // xc4
                                is_suffix = true;
                                break;
                            }
                        }

                        // if suffix was found
                        if (is_suffix) {
                            break;
                        }
                    }
                    size_bytes_len++;
                }
                break;
            }
        }

        // initialize
        int height;
        int width;

        // alloc width and height
        int div = size_bytes_len / 2;
        unsigned char *width_arr = (unsigned char *)malloc(sizeof(unsigned char) * div);
        unsigned char *height_arr = (unsigned char *)malloc(sizeof(unsigned char) * div);

        // if pos could not be found for either images or length of bytes are not equal, set width
        // and height to 1 in order to still anonymize image
        if (pos == -1 || size_bytes_len % 2 != 0) {
            height = 1;
            width = 1;
        } else {

            // set values
            for (int i = 0; i < div; i++) {
                width_arr[i] = decoded_data[pos + i];
                height_arr[i] = decoded_data[pos + div + i];
            }

            // convert bytes into int
            height = bytes_to_int(height_arr, div);
            width = bytes_to_int(width_arr, div);
        }

        // alloc with width and height and fill with 0 for a black image
        unsigned char *black_image = (unsigned char *)malloc(width * height);
        memset(black_image, 0, width * height);

        // create black jpg image
        jpec_enc_t *e = jpec_enc_new(black_image, width, height);
        int len;
        const uint8_t *jpeg = jpec_enc_run(e, &len);

        // encode new image data and check if string is longer than original string, replace old
        // base64-encoded string afterwards
        char *new_image_data = b64_encode(jpeg, strlen(image_data));
        if (strlen(new_image_data) > strlen(image_data)) {
            new_image_data[strlen(image_data)] = '\0';
        }
        result = replace_str(result, image_data, new_image_data);
        rewrite = true;

        // free memory and release encoder
        free(width_arr);
        free(height_arr);
        free(decoded_data);
        free(black_image);
        free(new_image_data);
        jpec_enc_del(e);
    }

    // alter XML header
    if (rewrite) {
        file_seek(fp, 0, SEEK_SET);
        if (!file_write(result, header_size, 1, fp)) {
            fprintf(stderr, "Error: changing XML Header failed.\n");
            free(buffer);
            return -1;
        }
    }

    free(buffer);
    return 1;
}

// anonymize iSyntax file
int32_t handle_isyntax(const char **filename, const char *new_label_name, bool keep_macro_image,
                       bool do_inplace) {

    fprintf(stdout, "Anonymize iSyntax WSI...\n");

    if (!do_inplace) {
        *filename = duplicate_file(*filename, new_label_name, DOT_ISYNTAX);
    }

    file_t *fp = file_open(*filename, "r+");

    // if file could not be opened
    if (fp == NULL) {
        return -1;
    }

    int32_t header_size = get_size_to_substring(fp, ISYNTAX_EOT);

    // remove label image
    int32_t result = wipe_image_data(fp, header_size, "LABELIMAGE");

    if (result == -1) {
        fprintf(stderr, "Error: Could not wipe label image from file.\n");
    }

    // remove macro image
    if (!keep_macro_image) {
        result = wipe_image_data(fp, header_size, "MACROIMAGE");
    }

    if (result == -1) {
        fprintf(stderr, "Error: Could not wipe macro image from file.\n");
    }

    anonymize_isyntax_metadata(fp, header_size);

    // clean up
    file_close(fp);
    return 1;
}