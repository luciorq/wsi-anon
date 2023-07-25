#include "wsi-anonymizer.h"

// TODO: remove this function
int32_t (*is_format_functions[])(const char *filename) = {&is_aperio,  &is_hamamatsu, &is_mirax,
                                                          &is_ventana, &is_isyntax,   &is_philips_tiff};

int32_t (*handle_format_functions[])(const char **filename, const char *new_label_name, bool keep_macro_image,
                                     bool disable_unlinking, bool do_inplace) = {
    &handle_aperio, &handle_hamamatsu, &handle_mirax, &handle_ventana, &handle_isyntax, &handle_philips_tiff};

int8_t num_of_formats = sizeof(VENDOR_AND_FORMAT_STRINGS) / sizeof(char *);

struct wsi_data *(*get_wsi_data_functions[])(const char *filename) = {
    &get_wsi_data_aperio,  &get_wsi_data_hamamatsu, &get_wsi_data_mirax,
    &get_wsi_data_ventana, &get_wsi_data_isyntax,   &get_wsi_data_philips_tiff};

struct wsi_data *get_wsi_data(const char *filename) {
    if (file_exists(filename)) {
        for (int8_t i = 0; i < num_of_formats - 2; i++) {
            struct wsi_data *wsi_data = get_wsi_data_functions[i](filename);
            if (wsi_data != NULL) {
                return wsi_data;
            }
        }
        // unknown format
        struct wsi_data *wsi_data = malloc(sizeof(*wsi_data));
        wsi_data->format = num_of_formats - 2;
        return wsi_data;
    } else {
        // invalid format
        struct wsi_data *wsi_data = malloc(sizeof(*wsi_data));
        wsi_data->format = num_of_formats - 1;
        return wsi_data;
    }
}

int32_t anonymize_wsi_with_result(const char **filename, const char *new_label_name, bool keep_macro_image,
                                  bool disable_unlinking, bool do_inplace) {
    int32_t result = -1;

    struct wsi_data *wsi_data = get_wsi_data(*filename);

    if (wsi_data->format == num_of_formats - 1) {
        fprintf(stderr, "Error: File does not exist or is invalid.\n");
        free(wsi_data);
        return result;
    } else if (wsi_data->format == num_of_formats - 2) {
        fprintf(stderr, "Error: Unknown file format. Process aborted.\n");
        free(wsi_data);
        return result;
    } else {
        result = handle_format_functions[wsi_data->format](filename, new_label_name, keep_macro_image,
                                                           disable_unlinking, do_inplace);
        free(wsi_data);
        return result;
    }
}

int32_t anonymize_wsi_inplace(const char *filename, const char *new_label_name, bool keep_macro_image,
                              bool disable_unlinking) {
    return anonymize_wsi_with_result(&filename, new_label_name, keep_macro_image, disable_unlinking, true);
}

int32_t anonymize_wsi(const char *filename, const char *new_label_name, bool keep_macro_image, bool disable_unlinking,
                      bool do_inplace) {
    return anonymize_wsi_with_result(&filename, new_label_name, keep_macro_image, disable_unlinking, do_inplace);
}

void freeMem(void *ptr) { free(ptr); }