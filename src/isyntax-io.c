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
    int32_t *size = ret - buffer;

    file_seek(fp, 0, SEEK_SET);
    free(buffer);

    return size;
}

// searches for specific value in XML file
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

// checks isyntax file format
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

// searches for attribute in XML Header replaces its value with equal amount of X's or 0
char *wipe_xml_data(char *result, char *attribute) {

    const char *value = get_string_between_delimiters(result, attribute, "<Attribute");
    const char *delimiter = get_string_between_delimiters(value, "PMSVR=", ">");
    char *replace_with;

    // check for datatype in XML Header
    if (strcmp(delimiter, ISYNTAX_DELIMITER_STR) == 0) {
        value = get_string_between_delimiters(value, concat_str(ISYNTAX_DELIMITER_STR, ">"),
                                              ISYNTAX_DELIMITER2);
        replace_with = "X";
    } else if (strcmp(delimiter, ISYNTAX_DELIMITER_INT) == 0) {
        value = get_string_between_delimiters(value, concat_str(ISYNTAX_DELIMITER_INT, ">"),
                                              ISYNTAX_DELIMITER2);
        replace_with = "0";
    } else {
        fprintf(stderr, "Cannot anonymize this datatype in XML Header");
        return result;
    }

    // check for empty String
    if (strcmp(value, "") != 0) {
        char *replacement = get_empty_string(replace_with, strlen(value));
        return replace_str(result, value, replacement);
    }

    return result;
}

// anonymizes metadata from isyntax file
int32_t anonymize_isyntax_metadata(file_t *fp) {

    // get rough size for metadata
    int32_t size = get_size_to_substring(fp, ISYNTAX_EOT);
    char *buffer = (char *)malloc(size);

    // file_seek(fp, 0, SEEK_SET);

    if (file_read(buffer, size, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read iSyntax file.\n");
        return -1;
    }

    char *result = buffer;
    bool rewrite = false;

    if (contains(result, ISYNTAX_DATETIME_ATT)) {
        result = wipe_xml_data(result, ISYNTAX_DATETIME_ATT);
        rewrite = true;
    }

    if (contains(result, ISYNTAX_SERIAL_ATT)) {
        result = wipe_xml_data(result, ISYNTAX_SERIAL_ATT);
        rewrite = true;
    }

    if (contains(result, ISYNTAX_RACK_ATT)) {
        result = wipe_xml_data(result, ISYNTAX_RACK_ATT);
        rewrite = true;
    }

    if (contains(result, ISYNTAX_SLOT_ATT)) {
        result = wipe_xml_data(result, ISYNTAX_SLOT_ATT);
        rewrite = true;
    }

    if (contains(result, ISYNTAX_OPERID_ATT)) {
        result = wipe_xml_data(result, ISYNTAX_OPERID_ATT);
        rewrite = true;
    }

    if (contains(result, ISYNTAX_BARCODE_ATT)) {
        result = wipe_xml_data(result, ISYNTAX_BARCODE_ATT);
        rewrite = true;
    }

    // alters XML Header of iSyntax file
    if (rewrite) {
        file_seek(fp, 0, SEEK_SET);
        if (!file_write(result, size, 1, fp)) {
            fprintf(stderr, "Error: changing XML Header failed.\n");
            free(buffer);
            return -1;
        }
    }

    free(buffer);
    return 1;
}

// ToDo: remove macro/label image from file
int32_t wipe_image_data(file_t *fp, char *image_type) {

    // gets only the xml header
    int32_t size = get_size_to_substring(fp, ISYNTAX_EOT);
    char *buffer = (char *)malloc(size);

    if (file_read(buffer, size, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read iSyntax file.\n");
        return 0;
    }

    char *result = buffer;
    bool rewrite = false;

    if (contains(result, image_type)) {

        // get image data string
        const char *image_data = get_string_between_delimiters(result, image_type, "Object>");
        image_data = get_string_between_delimiters(image_data, ISYNTAX_IMAGE_DATA, "</Data");
        image_data = get_string_between_delimiters(
            image_data, concat_str(ISYNTAX_DELIMITER_STR, ">"), ISYNTAX_DELIMITER2);

        // decode base64 string for image data
        size_t decode_size[1];
        decode_size[0] = strlen(image_data);
        unsigned char *decoded_data = b64_decode_ex(image_data, decode_size, &decode_size);

        // offset and length of bytes of width and height
        size_t pos = -1;
        size_t size_bytes_len = 0;

        // check structure \xff\xc0\x00\x11\x08<width><height>\x03\x01.....\xff\xc4 for width and
        // height
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

        // the length of bytes of width and height are always the same => length % 2 == 0
        if (pos == -1 || size_bytes_len % 2 != 0) {
            printf(stderr, "Width and height of image can not be found");
            return -1;
        }

        // get width and height
        int div = size_bytes_len / 2;
        unsigned char *width_str = (unsigned char *)malloc(sizeof(unsigned char) * div);
        unsigned char *height_str = (unsigned char *)malloc(sizeof(unsigned char) * div);

        for (int i = 0; i < div; i++) {
            width_str[i] = (unsigned char *)decoded_data[pos + i];
            height_str[i] = (unsigned char *)decoded_data[pos + div + i];
        }

        // ToDo: create anonymized image
        unsigned char *new_img = (unsigned char *)malloc(sizeof(decoded_data));
        create_black_image(new_img, width_str, height_str);

        // free memory
        free(width_str);
        free(height_str);
        free(decoded_data);

        return 1;
    }

    // changes image data in XML Header of iSyntax file
    if (rewrite) {
        file_seek(fp, 0, SEEK_SET);
        if (!file_write(result, size, 1, fp)) {
            fprintf(stderr, "Error: changing XML Header failed.\n");
            free(buffer);
            return -1;
        }
    }

    free(buffer);
    return 1;
}

// anonymize isyntax file format
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

    // ToDo: remove label image
    int32_t result = wipe_image_data(fp, "LABELIMAGE");

    if (!result) {
        fprintf(stderr, "Error: Could not wipe label image from file.\n");
        return -1;
    }
    /*
        if (!keep_macro_image) {
            // ToDo: remove macro image
            int32_t result = wipe_image_data(fp, "MACROIMAGE");
        }

        if (!result) {
            fprintf(stderr, "Error: Could not wipe macro image from file.\n");
            return -1;
        }*/

    anonymize_isyntax_metadata(fp);

    // clean up
    file_close(fp);
    return 1;
}

// ToDo: Refactor

static int current_pos;

unsigned char *create_black_image(unsigned char *new_image, unsigned char *width_str,
                                  unsigned char *height_str) {
    add_soi(new_image);
    add_app(new_image);
    // add the rest here
    add_eoi(new_image);
}

// add start of image
static void add_soi(unsigned char *new_image) { add_segment(new_image, "\xff\xd8"); }

// --- CONTINUE HERE ---

// add application 0 default header
static void add_app(unsigned char *new_image) {
    add_segment(new_image, "\xff\xe0");
    add_segment(new_image, "\x00\x10JFIF");
    add_segment(new_image, "\x00\x01");
    add_segment(new_image, "\x01\x01");
    add_segment(new_image, "\x00\x48");
    add_segment(new_image, "\x00\x48");
    add_segment(new_image, "\x00\x00");
}

// add end of image
static void add_eoi(unsigned char *new_image) { add_segment(new_image, "\xff\xd9"); }

// add segment to buffer
static void add_segment(unsigned char *new_image, unsigned char *data) {
    int new_pos;
    for (int i = 0; i < strlen(data); i++) {
        new_image[current_pos + i] = data[i];
        new_pos = current_pos + i;
    }
    current_pos = ++new_pos;
}
