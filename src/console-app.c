#include "defines.h"
#include "wsi-anonymizer.h"

static const char *VENDOR_STRINGS[] = {
    "Aperio", "Hamamatsu", "3DHistech (Mirax)", "Unknown", "Invalid"
};

int main(int argc, char *argv[]) {   
    bool only_check = false;
    bool unlink_directory = false;
    char *filename = NULL;
    char *new_label_name = NULL;
    printf("args %i\n", argc);

    size_t optind;
    for (optind = 1; optind < argc; optind++) {
        if(argv[optind][0] == '-') {
            switch (argv[optind][1]) {
                case 'c': {
                    only_check = true;
                    filename = argv[optind+1];
                    break;
                }
                case 'i': {
                    filename = argv[optind+1];
                    break;
                } 
                case 'n': {
                    printf("test");
                    new_label_name = argv[optind+1];
                    break;
                }
                case 'u': {
                    unlink_directory = true; 
                    break;
                }
                case 'h':
                default: {
                    fprintf(stderr, "Usage: %s [-ilw] [file...]\n", argv[0]);
                    exit(EXIT_FAILURE);
                }
            }
        }
           
    }

    if(only_check) {
        if(filename != NULL) {
            file_format format = check_file_format(filename);
            fprintf(stdout, "Vendor: [%s]\n", VENDOR_STRINGS[format]);
        } else {
            fprintf(stderr, "No filename to check for vendor selected.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        if(filename != NULL) {
            if(new_label_name != NULL) {
                anonymize_wsi(filename, new_label_name, unlink_directory);
            } else {
                //TODO
                anonymize_wsi(filename, "", unlink_directory);
            }
        } else {
            fprintf(stderr, "No file for anonymization selected.\n");
            exit(EXIT_FAILURE);
        }
    }

}   