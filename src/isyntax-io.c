#include "isyntax-io.h"

static const char ISYNTAX_EXT[] = "isyntax";
static const char DOT_ISYNTAX[] = ".isyntax";

// checks iSyntax file format
int32_t is_isyntax(const char *filename) {

    const char *ext = get_filename_ext(filename);

    // check for file extension
    if (strcmp(ext, ISYNTAX_EXT) != 0) {
        return 0;
    } else {
        int32_t result = 0;
        file_t *fp = file_open(filename, "rb+");

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

    // alters iSyntax file
    if (rewrite) {
        strcpy(buffer, result);
        file_seek(fp, 0, SEEK_SET);
        if (!file_write(buffer, header_size, 1, fp)) {
            fprintf(stderr, "Error: changing XML Header failed.\n");
            free(buffer);
            return -1;
        }
    }

    free(buffer);
    return 1;
}

// remove label image and macro image
int32_t wipe_isyntax_image_data(file_t *fp, size_t header_size, char *image_type) {

    // gets only the xml header
    char *buffer = (char *)malloc(header_size);

    file_seek(fp, 0, SEEK_SET);

    if (file_read(buffer, header_size, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read iSyntax file.\n");
        return -1;
    }

    // handle bigger overhead when using malloc in WASM
    if (strlen(buffer) > header_size) {
        buffer[header_size] = '\0';
    }

    char *result = buffer;
    bool rewrite = false;

    if (contains(result, image_type)) {

        // get image data string
        const char *image_data = get_string_between_delimiters(result, image_type, PHILIPS_OBJECT);
        image_data = get_string_between_delimiters(image_data, PHILIPS_IMAGE_DATA, ISYNTAX_DATA);
        image_data = get_string_between_delimiters(
            image_data, concat_str(PHILIPS_DELIMITER_STR, PHILIPS_CLOSING_SYMBOL), PHILIPS_ATT_END);

        // set height and width to 1
        int32_t height = 1;
        int32_t width = 1;

        // remove comments in order to set height and width to actual dimensions of image
        /*
        int32_t *dim = get_height_and_width(image_data);
        height = dim[0];
        width = dim[1];
        free(dim);
        */

        // alloc with height and width and fill with 255 for a white image
        unsigned char *white_image = (unsigned char *)malloc((height * width) * sizeof(unsigned char));
        memset(white_image, 255, height * width);

        // create white jpg image
        jpec_enc_t *e = jpec_enc_new(white_image, width, height);
        int32_t len;
        const uint8_t *jpeg = jpec_enc_run(e, &len);

        // encode new image data and check if string is longer than original string, replace old
        // base64-encoded string afterwards
        char *new_image_data = (char *)b64_encode(jpeg, len);
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

    // alter XML header
    if (rewrite) {
        strcpy(buffer, result);
        file_seek(fp, 0, SEEK_SET);
        if (!file_write(buffer, header_size, 1, fp)) {
            fprintf(stderr, "Error: changing XML Header failed.\n");
            result = NULL;
            free(buffer);
            return -1;
        }
    }
    result = NULL;
    free(buffer);
    return 0;
}

// anonymize iSyntax file
int32_t handle_isyntax(const char **filename, const char *new_label_name, bool keep_macro_image, bool disable_unlinking,
                       bool do_inplace) {

    if (disable_unlinking) {
        fprintf(stderr, "Error: Cannot disable unlinking in iSyntax file.\n");
    }

    fprintf(stdout, "Anonymize iSyntax WSI...\n");

    if (!do_inplace) {
        *filename = duplicate_file(*filename, new_label_name, DOT_ISYNTAX);
    }

    file_t *fp = file_open(*filename, "rb+");

    // if file could not be opened
    if (fp == NULL) {
        return -1;
    }

    size_t header_size = get_size_to_substring(fp, ISYNTAX_EOT);

    // remove label image
    int32_t result = wipe_isyntax_image_data(fp, header_size, PHILIPS_LABELIMAGE);

    if (result == -1) {
        fprintf(stderr, "Error: Could not wipe label image from file.\n");
    }

    // remove macro image
    if (!keep_macro_image) {
        result = wipe_isyntax_image_data(fp, header_size, PHILIPS_MACROIMAGE);

        if (result == -1) {
            fprintf(stderr, "Error: Could not wipe macro image from file.\n");
        }
    }

    anonymize_isyntax_metadata(fp, header_size);

    // clean up
    file_close(fp);
    return result;
}