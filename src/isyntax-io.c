#include "isyntax-io.h"

// TODO: remove this
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
            fprintf(stderr, "Error: Could not open iSyntax file.\n");
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

struct metadata_attribute *get_attribute_isyntax(char *buffer, char *attribute) {
    const char *value = get_value_from_attribute(buffer, attribute);
    // check if value of attribute is not an empty string
    if (value[0] != '\0') {
        struct metadata_attribute *single_attribute = malloc(sizeof(*single_attribute));
        single_attribute->key = attribute;
        single_attribute->value = strdup(value);
        return single_attribute;
    }
    return NULL;
}

struct metadata *get_metadata_isyntax(file_t *fp, int32_t header_size) {
    // read content of XML header into buffer
    char *buffer = malloc(header_size);
    file_seek(fp, 0, SEEK_SET);
    if (file_read(buffer, header_size, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read XML header of iSyntax file.\n");
        return NULL;
    }

    // all metadata
    static char *METADATA_ATTRIBUTES[] = {PHILIPS_DATETIME_ATT, PHILIPS_SERIAL_ATT, PHILIPS_SLOT_ATT,
                                          PHILIPS_RACK_ATT,     PHILIPS_OPERID_ATT, PHILIPS_BARCODE_ATT};

    // initialize metadata_attribute struct
    struct metadata_attribute **attributes =
        malloc(sizeof(**attributes) * sizeof(METADATA_ATTRIBUTES) / sizeof(METADATA_ATTRIBUTES[0]));
    int8_t metadata_id = 0;

    // checks for all metadata
    for (size_t i = 0; i < sizeof(METADATA_ATTRIBUTES) / sizeof(METADATA_ATTRIBUTES[0]); i++) {
        if (contains(buffer, METADATA_ATTRIBUTES[i])) {
            struct metadata_attribute *single_attribute = get_attribute_isyntax(buffer, METADATA_ATTRIBUTES[i]);
            if (single_attribute != NULL) {
                attributes[metadata_id++] = single_attribute;
            }
        }
    }

    // add all found metadata
    free(buffer);
    struct metadata *metadata_attributes = malloc(sizeof(*metadata_attributes));
    metadata_attributes->attributes = attributes;
    metadata_attributes->length = metadata_id;
    return metadata_attributes;
}

struct wsi_data *get_wsi_data_isyntax(const char *filename) {
    // gets file extension
    int32_t result = 0;
    const char *ext = get_filename_ext(filename);

    // check for valid file extension
    if (strcmp(ext, ISYNTAX_EXT) != 0) {
        return NULL;
    }

    // opens file
    file_t *fp = file_open(filename, "rb+");

    // check if file was successfully opened
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open iSyntax file.\n");
        return NULL;
    }

    // checks root node in order to determine if file is actually iSyntax
    result = file_contains_value(fp, ISYNTAX_ROOTNODE);

    // checks result
    if (result == -1) {
        fprintf(stderr, "Error: Could not find root node in iSyntax file.\n");
        return NULL;
    }

    // get header size
    size_t header_size = get_size_to_substring(fp, ISYNTAX_EOT);

    // gets all metadata
    struct metadata *metadata_attributes = get_metadata_isyntax(fp, header_size);

    // is iSyntax
    // TODO: replace format value and handle more efficiently
    struct wsi_data *wsi_data = malloc(sizeof(*wsi_data));
    wsi_data->format = 4;
    wsi_data->filename = filename;
    wsi_data->metadata_attributes = metadata_attributes;

    // TODO: find bug corrupted size vs. prez_size

    // cleanup
    file_close(fp);
    return wsi_data;
}

// TODO: make use of get_wsi_data_isyntax function
// anonymizes metadata from iSyntax file
int32_t anonymize_isyntax_metadata(file_t *fp, int32_t header_size, const char *pseudonym) {

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
        result = anonymize_value_of_attribute(result, PHILIPS_SERIAL_ATT, pseudonym);
        rewrite = true;
    }

    // wipes complete section of given attribute
    if (contains(result, PHILIPS_SLOT_ATT)) {
        result = wipe_section_of_attribute(
            result, PHILIPS_SLOT_ATT); // ToDo: check if pseudonym can be used here instead of blanks
        rewrite = true;
    }

    // wipes complete section of given attribute
    if (contains(result, PHILIPS_RACK_ATT)) {
        result = wipe_section_of_attribute(
            result, PHILIPS_RACK_ATT); // ToDo: check if pseudonym can be used here instead of blanks
        rewrite = true;
    }

    // replace with arbitrary value
    if (contains(result, PHILIPS_OPERID_ATT)) {
        result = anonymize_value_of_attribute(result, PHILIPS_OPERID_ATT, pseudonym);
        rewrite = true;
    }

    // replace with arbitrary value
    if (contains(result, PHILIPS_BARCODE_ATT)) {
        result = anonymize_value_of_attribute(result, PHILIPS_BARCODE_ATT, pseudonym);
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
int32_t handle_isyntax(const char **filename, const char *new_filename, const char *pseudonym_metadata,
                       struct anon_configuration *configuration) {

    if (configuration->disable_unlinking) {
        fprintf(stderr, "Error: Cannot disable unlinking in iSyntax file.\n");
    }

    fprintf(stdout, "Anonymizing iSyntax WSI...\n");

    if (!configuration->do_inplace) {
        *filename = duplicate_file(*filename, new_filename, DOT_ISYNTAX);
    }

    file_t *fp = file_open(*filename, "rb+");

    // if file could not be opened
    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open iSyntax file.\n");
        return -1;
    }

    size_t header_size = get_size_to_substring(fp, ISYNTAX_EOT);

    // remove label image
    int32_t result = wipe_isyntax_image_data(fp, header_size, PHILIPS_LABELIMAGE);

    if (result == -1) {
        fprintf(stderr, "Error: Could not wipe label image from file.\n");
    }

    // remove macro image
    if (!configuration->keep_macro_image) {
        result = wipe_isyntax_image_data(fp, header_size, PHILIPS_MACROIMAGE);

        if (result == -1) {
            fprintf(stderr, "Error: Could not wipe macro image from file.\n");
        }
    }

    anonymize_isyntax_metadata(fp, header_size, pseudonym_metadata);

    // clean up
    file_close(fp);
    return result;
}