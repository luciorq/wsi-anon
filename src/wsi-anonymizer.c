#include "wsi-anonymizer.h"

file_format check_file_format(const char *filename) {
    fprintf(stdout, "Checking file format...\n");
    if (file_exists(filename) == 1) {
        if (is_aperio(filename)) {
            return aperio_svs;
        } else if (is_hamamatsu(filename)) {
            return hamamatsu_ndpi;
        } else if (is_mirax(filename)) {
            return histech_mirax;
        } else if (is_ventana(filename)) {
            return ventana;
        } else {
            return unknown_format;
        }
    } else {
        return invalid;
    }
}

int32_t anonymize_wsi_with_result(const char **filename, const char *new_label_name,
                                  bool keep_macro_image, bool disbale_unlinking, bool do_inplace) {
    int32_t result = -1;
    switch (check_file_format(*filename)) {
    case aperio_svs: {
        result = handle_aperio(filename, new_label_name, keep_macro_image, disbale_unlinking,
                               do_inplace);
        break;
    }
    case hamamatsu_ndpi: {
        result = handle_hamamatsu(filename, new_label_name, disbale_unlinking, do_inplace);
        break;
    }
    case histech_mirax: {
        result =
            handle_mirax(filename, new_label_name, keep_macro_image, disbale_unlinking, do_inplace);
        break;
    }
    case ventana: {
        if(keep_macro_image){
            fprintf(stderr, "Error: Cannot keep macro image in ventana file.\n");
        }
        result = handle_ventana(filename, new_label_name, disbale_unlinking, do_inplace);
        break;
    }
    case unknown_format: {
        fprintf(stderr, "Error: Unknown file format. Process aborted.\n");
        break;
    }
    case invalid: {
        fprintf(stderr, "Error: File does not exist or is invalid.\n");
        break;
    }
    }
    return result;
}

int32_t anonymize_wsi_inplace(const char *filename, const char *new_label_name,
                              bool keep_macro_image, bool disbale_unlinking) {
    return anonymize_wsi_with_result(&filename, new_label_name, keep_macro_image, disbale_unlinking,
                                     true);
}

const char *anonymize_wsi(const char *filename, const char *new_label_name, bool keep_macro_image,
                          bool disbale_unlinking, bool do_inplace) {
    anonymize_wsi_with_result(&filename, new_label_name, keep_macro_image, disbale_unlinking,
                              do_inplace);
    return filename;
}

void freeMem(void *ptr) { free(ptr); }