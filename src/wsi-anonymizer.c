#include "wsi-anonymizer.h"

int32_t (*is_format_functions[])(const char *filename) = {&is_aperio, &is_hamamatsu, &is_mirax,
                                                          &is_ventana, &is_isyntax};

int32_t (*handle_format_functions[])(const char **filename, const char *new_label_name,
                                     bool keep_macro_image, bool disable_unlinking,
                                     bool do_inplace) = {
    &handle_aperio, &handle_hamamatsu, &handle_mirax, &handle_ventana, &handle_isyntax};

int8_t num_of_formats = sizeof(VENDOR_STRINGS) / sizeof(char *);

int8_t check_file_format(const char *filename) {

    if (file_exists(filename) == 1) {
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

int32_t anonymize_wsi_with_result(const char **filename, const char *new_label_name,
                                  bool keep_macro_image, bool disable_unlinking, bool do_inplace) {
    int32_t result = -1;

    int8_t result_check_format = check_file_format(*filename);

    if (result_check_format == num_of_formats - 1) {
        fprintf(stderr, "Error: File does not exist or is invalid.\n");
        return result;
    } else if (result_check_format == num_of_formats - 2) {
        fprintf(stderr, "Error: Unknown file format. Process aborted.\n");
        return result;
    } else {
        result = handle_format_functions[result_check_format](
            filename, new_label_name, keep_macro_image, disable_unlinking, do_inplace);
        return result;
    }
}

int32_t anonymize_wsi_inplace(const char *filename, const char *new_label_name,
                              bool keep_macro_image, bool disable_unlinking) {
    return anonymize_wsi_with_result(&filename, new_label_name, keep_macro_image, disable_unlinking,
                                     true);
}

const char *anonymize_wsi(const char *filename, const char *new_label_name, bool keep_macro_image,
                          bool disable_unlinking, bool do_inplace) {
    anonymize_wsi_with_result(&filename, new_label_name, keep_macro_image, disable_unlinking,
                              do_inplace);
    return filename;
}

void freeMem(void *ptr) { free(ptr); }