#include "tiff-based-io.h"

typedef enum file_format {
    aperio_svs,
    hamamatsu_ndpi,
    histech_mirax,
    unknown_format,
    invalid
} file_format;

file_format check_file_format(const char *filename) {
    if(file_exists(filename) == 1) {
        if(is_aperio(filename)) {
            return aperio_svs;
        } else if(is_hamamatsu(filename)) {
            return hamamatsu_ndpi;
        } else if (is_mirax(filename)) {
            return histech_mirax;
        } else {
            return unknown_format;
        }
    } else {
        return invalid;
    }
}

const char* anonymize_wsi(const char *filename, const char *new_label_name) {
    switch(check_file_format(filename)){
        case aperio_svs: handle_aperio(filename, new_label_name); break;
        case hamamatsu_ndpi: handle_hamamatsu(filename, new_label_name); break;
        case histech_mirax: handle_mirax(filename, new_label_name); break;
        case unknown_format: printf("Unknown file format. Process aborted."); break;
        case invalid: printf("File does not exist or is invalid.");
    }
}

int main() {
    // this is solely for testing/debugging
    char *filename = "//home//mfranz//Documents//other//test_wsi-anonymizer//test.ndpi";

    const char *new_filename = anonymize_wsi(filename, "new_test_label");
}