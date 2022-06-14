#include "isyntax-io.h"

// searches for specific value in XML file
int32_t file_contains_value(file_t *fp, char *value, int size) {

    file_seek(fp, 0, SEEK_SET);
    char *buffer = (char *)malloc(size);
                                                                                                                       
    if (file_read(buffer, size, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read iSyntax file.\n");
        return 0;
    }

    // searches for value
    if (contains(buffer, value)){
        free(buffer);
        return 1;
    }

    free(buffer);
    return 0;
}

// checks isyntax file format
int32_t is_isyntax(const char *filename){

    const char *ext = get_filename_ext(filename);

    // check for file extension
    if(strcmp(ext, ISYNTAX_EXT) != 0) {
        return 0;
    } else {
        int32_t result = 0;
        file_t *fp = file_open(filename, "r");

        // if file could not be opened
        if (fp == NULL) {
            return result;
        }

        // searches for root node in file for verification
        result = file_contains_value(fp, ISYNTAX_ROOTNODE, ISYNTAX_ROOTNODE_SIZE);

        // if root node could not be found
        if (!result) {
            return result;
        }

        // is iSyntax
        file_close(fp);
        return result;
    } 
}

// searches for attribute in XML Header replaces its value with equal amount of X's or 0 
char *wipe_xml_data(char *result, char *attribute) {

    const char *value = get_string_between_delimiters(result, attribute, "<Attribute");
    const char *delimiter = get_string_between_delimiters(value, "PMSVR=", ">");
    char *replace_with;

    // check for datatype in XML Header
    if(strcmp(delimiter, ISYNTAX_DELIMITER_STR) == 0){
        value = get_string_between_delimiters(value, concat_str(ISYNTAX_DELIMITER_STR, ">"), ISYNTAX_DELIMITER2);
        replace_with = "X";
    } else if(strcmp(delimiter, ISYNTAX_DELIMITER_INT) == 0){
        value = get_string_between_delimiters(value, concat_str(ISYNTAX_DELIMITER_INT, ">"), ISYNTAX_DELIMITER2);
        replace_with = "0";
    } else {
        fprintf(stderr, "Cannot anonymize this datatype in XML Header");
        return result;
    }

    // check for empty String
    if(strcmp(value, "") != 0){
        char *replacement = get_empty_string(replace_with, strlen(value));
        return replace_str(result, value, replacement);
    }

    return result;
}

// anonymizes metadata from isyntax file
int32_t anonymize_isyntax_metadata(file_t *fp){

    int size = 2000;                                                // ToDo: find exact size of xml header in and more reliable way
    char *buffer = (char *)malloc(size);

    file_seek(fp, 0, ISYNTAX_ROOTNODE_SIZE);
                                                                                                                       
    if (file_read(buffer, size, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read iSyntax file.\n");
        return -1;
    }

    char *result = buffer;
    bool rewrite = false;
    
    if(contains(result, ISYNTAX_DATETIME_ATT)){
        result = wipe_xml_data(result, ISYNTAX_DATETIME_ATT);
        rewrite = true;
    }

    if(contains(result, ISYNTAX_SERIAL_ATT)){
        result = wipe_xml_data(result, ISYNTAX_SERIAL_ATT);
        rewrite = true;
    }

    if(contains(result, ISYNTAX_RACK_ATT)){
        result = wipe_xml_data(result, ISYNTAX_RACK_ATT);
        rewrite = true;
    }

    if(contains(result, ISYNTAX_SLOT_ATT)){
        result = wipe_xml_data(result, ISYNTAX_SLOT_ATT);
        rewrite = true;
    }

    if(contains(result, ISYNTAX_OPERID_ATT)){
        result = wipe_xml_data(result, ISYNTAX_OPERID_ATT);
        rewrite = true;
    }

    if(contains(result, ISYNTAX_BARCODE_ATT)){
        result = wipe_xml_data(result, ISYNTAX_BARCODE_ATT);
        rewrite = true;
    }

    // alters XML Header of isyntax file
    if (rewrite) {
        file_seek(fp, 0, ISYNTAX_ROOTNODE_SIZE);
        if (!file_write(result, size, 1, fp)) {
            fprintf(stderr, "Error: changing XML Header failed.\n");
            free(buffer);
            return -1;
        }
    }

    free(buffer);
    return 1;
}

// ToDo: remove/black out label image from codeblock in file 
int32_t wipe_label_image(file_t *fp){
    int size = 1850;                                                // ToDo: find exact size of xml header in a more reliable way
    char *buffer = (char *)malloc(size);

    file_seek(fp, 0, size);
                                                                                                                       
    if (file_read(buffer, 512, 1, fp) != 1) {
        free(buffer);
        fprintf(stderr, "Error: Could not read iSyntax file.\n");
        return 0;
    }

    fprintf(stdout, buffer);
    
    free(buffer);
    return 1;
}

// ToDo : continue WSI anonymization
int32_t handle_isyntax(const char **filename, const char *new_label_name, bool keep_macro_image, 
                        bool do_inplace){

    fprintf(stdout, "Anonymize iSyntax WSI...\n");

    if (!do_inplace) {
        *filename = duplicate_file(*filename, new_label_name, DOT_ISYNTAX);
    }
    
    file_t *fp = file_open(*filename, "r+");

    // if file could not be opened
    if (fp == NULL) {
        return -1;
    }

    // --- BEGINNING OF ACTUAL ANONYMIZATION OF FILE HERE ---

    // ToDo: remove label image
    int32_t result = wipe_label_image(fp);

    if(!result){
        fprintf(stderr, "Error: Could not wipe label image from file.\n");
        return -1;
    }

    if(!keep_macro_image){
        // ToDo: remove macro image
    }

    //anonymize_isyntax_metadata(fp);
        
    // clean up
    file_close(fp);
    return 1;
}



/*
DONE:
- implemented first steps for isyntax anonymization
- additionally check header for verification of file
- added docs
- added anonymization of metadata
*/




