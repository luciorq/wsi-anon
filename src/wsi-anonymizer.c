#include "wsi-anonymizer.h"

int32_t (*is_format_functions[])(const char *filename) = {&is_aperio,  &is_hamamatsu, &is_mirax,
                                                          &is_ventana, &is_isyntax,   &is_philips_tiff};

int32_t (*handle_format_functions[])(const char **filename, const char *new_filename, const char *pseudonym_metadata,
                                     struct anon_configuration *configuration) = {
    &handle_aperio, &handle_hamamatsu, &handle_mirax, &handle_ventana, &handle_isyntax, &handle_philips_tiff};

struct wsi_data *(*get_wsi_data_functions[])(const char *filename) = {&get_wsi_data_aperio};

int8_t num_of_formats = sizeof(VENDOR_AND_FORMAT_STRINGS) / sizeof(char *);

int8_t check_file_format(const char *filename) {
    if (file_exists(filename)) {
        for (int8_t i = 0; i < num_of_formats - 2; i++) {
            if (is_format_functions[i](filename)) {
                return i;
            }
        }
        // unknown format
        return num_of_formats - 2;
    } else {
        // invalid format
        return num_of_formats - 1;
    }
}

struct wsi_data *get_wsi_data(const char *filename) {
    // TODO: create function pointer array for metadata and run for every format
    if (file_exists(filename)) {
        for (int8_t i = 0; i < num_of_formats - 2; i++) {
            struct wsi_data *wsi_data = get_wsi_data_functions[i](filename);
            if (wsi_data != NULL) {
                return wsi_data;
            }
        }
        // unknown format
        // return num_of_formats - 2;
        struct wsi_data *wsi_data = malloc(sizeof(*wsi_data));
        wsi_data->format = num_of_formats - 2;
        return wsi_data;
    } else {
        // invalid format
        // return num_of_formats - 1;
        struct wsi_data *wsi_data = malloc(sizeof(*wsi_data));
        wsi_data->format = num_of_formats - 1;
        return wsi_data;
    }
}

int32_t anonymize_wsi_with_result(const char **filename, const char *new_filename, const char *pseudonym_metadata,
                                  struct anon_configuration *configuration) {

    int32_t result = -1;

    int8_t result_check_format = check_file_format(*filename);

    if (result_check_format == num_of_formats - 1) {
        fprintf(stderr, "Error: File does not exist or is invalid.\n");
        return result;
    } else if (result_check_format == num_of_formats - 2) {
        fprintf(stderr, "Error: Unknown file format. Process aborted.\n");
        return result;
    } else {
        result =
            handle_format_functions[result_check_format](filename, new_filename, pseudonym_metadata, configuration);
        return result;
    }
}

int32_t anonymize_wsi_inplace(const char *filename, const char *new_filename, const char *pseudonym_metadata,
                              bool keep_macro_image, bool disable_unlinking) {
    // TODO: replace the first two values of struct accordingly
    struct anon_configuration *configuration = malloc(sizeof(*configuration));
    configuration->overwrite_label = false;
    configuration->overwrite_macro = false;
    configuration->overwrite_metadata = strcmp(pseudonym_metadata, "X") == 0 ? false : true;
    configuration->keep_macro_image = keep_macro_image;
    configuration->disable_unlinking = disable_unlinking;
    configuration->do_inplace = true;
    return anonymize_wsi_with_result(&filename, new_filename, pseudonym_metadata, configuration);
}

int32_t anonymize_wsi(const char *filename, const char *new_filename, const char *pseudonym_metadata,
                      struct anon_configuration *configuration) {
    return anonymize_wsi_with_result(&filename, new_filename, pseudonym_metadata, configuration);
}