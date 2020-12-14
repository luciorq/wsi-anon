#include "wsi-anonymizer.h"

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
        case aperio_svs: {
            handle_aperio(filename, new_label_name, unlink_directory); 
            break;
        }
        case hamamatsu_ndpi: {
            handle_hamamatsu(filename, new_label_name, unlink_directory); 
            break;
        }
        case histech_mirax: {
            handle_mirax(filename, new_label_name, unlink_directory); 
            break;
        }
        case unknown_format: { 
            fprintf(stderr, "Error: Unknown file format. Process aborted."); 
            break;
        }
        case invalid: {
            fprintf(stderr, "Error: File does not exist or is invalid.");
        }
    }
    // todo: return full filename and path
    return new_label_name;
}