#include "wsi-anonymizer.h"

// get correct length of char array
const char *file_formats = {"is_aperio", "is_hamamatsu", "is_mirax", "is_ventana", "is_isyntax", "unknown_format", "invalid"}; 
int8_t num_of_formats = sizeof(file_format);

int8_t check_file_format(const char *filename) {

    fprintf(stdout, "length of enum:%d\n", num_of_formats);

    // are the & correct?
    int32_t (*is_format_functions[])(const char *filename) = {&is_aperio, &is_hamamatsu, &is_mirax, &is_ventana, &is_isyntax};

    // call functions in functions array up to -2
    if (file_exists(filename) == 1){
        for(int8_t i = 0; i < num_of_formats; i++){
            if(is_format_functions[i]){
                return i;
                //return file_formats[num_of_formats];
            }
        }
        return file_formats[num_of_formats-2];
    } else {
        return file_formats[num_of_formats-1];
    }
}

// ToDo: switch-case in a for loop based on list above 
// handle different number of parameters in handle_format function
int32_t anonymize_wsi_with_result(const char **filename, const char *new_label_name,
                                  bool keep_macro_image, bool disable_unlinking, bool do_inplace) {
    int32_t result = -1;

    int32_t (*handle_format_functions[])(const char **filename, const char *new_label_name, bool keep_macro_image, bool disable_unlinking, bool do_inplace) = 
    {&handle_aperio, &handle_hamamatsu, &handle_mirax, &handle_ventana, &handle_isyntax};

    int8_t result_check_format = check_file_format(*filename);

    if (result_check_format == file_formats[num_of_formats-1]){
        fprintf(stderr, "Error: File does not exist or is invalid.\n");
        return result;
    }
    else if (result_check_format == file_formats[num_of_formats-2]){
        fprintf(stderr, "Error: Unknown file format. Process aborted.\n");
        return result;
    }
    else{
        result = handle_format_functions[result_check_format];
        return result;
    }

/*
    switch (check_file_format(*filename)) {
    case "is_aperio": {
        result = handle_aperio(filename, new_label_name, keep_macro_image, disable_unlinking,
                               do_inplace);
        break;
    }
    case "is_hamamatsu": {
        result = handle_hamamatsu(filename, new_label_name, disable_unlinking, do_inplace);
        break;
    }
    case "is_mirax": {
        result =
            handle_mirax(filename, new_label_name, keep_macro_image, disable_unlinking, do_inplace);
        break;
    }
    case "is_ventana": {
        if (keep_macro_image) {
            fprintf(stderr, "Error: Cannot keep macro image in Ventana file.\n");
        }
        result = handle_ventana(filename, new_label_name, disable_unlinking, do_inplace);
        break;
    }
    case "is_isyntax": {
        if (disable_unlinking) {
            fprintf(stderr, "Error: Cannot disable unlinking in iSyntax file.\n");
        }
        result = handle_isyntax(filename, new_label_name, keep_macro_image, do_inplace);
        break;
    }
    case "unknown_format": {
        fprintf(stderr, "Error: Unknown file format. Process aborted.\n");
        break;
    }
    case "invalid": {
        fprintf(stderr, "Error: File does not exist or is invalid.\n");
        break;
    }
    }
    return result;
    */
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