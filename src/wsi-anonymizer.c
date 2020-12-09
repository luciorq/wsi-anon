#include "wsi-anonymizer.h"
#include "tiff-based-io.h"

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

const char* anonymize_wsi(const char *filename, 
        const char *new_label_name,
        bool unlink_directory) {
    switch(check_file_format(filename)){
        case aperio_svs: handle_aperio(filename, new_label_name, unlink_directory); break;
        case hamamatsu_ndpi: handle_hamamatsu(filename, new_label_name, unlink_directory); break;
        case histech_mirax: handle_mirax(filename, new_label_name, unlink_directory); break;
        case unknown_format: printf("Unknown file format. Process aborted."); break;
        case invalid: printf("File does not exist or is invalid.");
    }
    // todo: return full filename and path
    return new_label_name;
}

/*int main() {
    // this is solely for testing/debugging
    //const char *filename = "//home//mfranz//Documents//other//test_wsi-anonymizer//test.mrxs";
    const char *filename = "//home//michael//Documents//other//test_data//CMU-1.mrxs";

    const char *new_filename = anonymize_wsi(filename, "new_test_label", true);

    printf("done... %s\n", new_filename);

    return EXIT_SUCCESS;
}*/