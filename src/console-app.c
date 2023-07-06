#include "wsi-anonymizer.h"

char *get_app_name() {
#ifdef __linux__
    return "bin/wsi-anon.out";
#else
    return "exe\\wsi-anon.exe";
#endif
}

void print_help_message() {
    fprintf(stderr, "Usage: %s [FILE] [-OPTIONS]\n\n", get_app_name());
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "-c     Only check file for vendor format\n");
    fprintf(stderr, "-n     Specify new filename (e.g. -n \"NEW_FILENAME\")\n");
    fprintf(stderr, "-p     Specify pseudonym letter for all metadata (e.g. -p \"X\")\n");
    fprintf(stderr, "-m     If flag is set, macro image will NOT be deleted\n");
    fprintf(stderr, "-i     If flag is set, anonymization will be done in-place\n");
    fprintf(stderr, "-u     If flag is set, tiff directory will NOT be unlinked\n\n");
    fprintf(stderr, "       Note: For file formats using JPEG compression this does not work currently.\n\n");
}

int32_t main(int32_t argc, char *argv[]) {
    bool only_check = false;
    bool keep_macro_image = false;
    bool disable_unlinking = false;
    bool do_inplace = false;
    const char *filename = NULL;
    const char *new_filename = NULL;
    const char *pseudonym_metadata = "X";

    if (argv[1] == NULL) {
        fprintf(stderr, "No filename specified.\n\n");
        print_help_message();
        exit(EXIT_FAILURE);
    }

    filename = argv[1];

    if (strcmp(filename, "-h\0") == 0 || strcmp(filename, "--help\0") == 0) {
        print_help_message();
        exit(EXIT_FAILURE);
    }

    for (int32_t optind = 2; optind < argc; optind++) {
        if (argv[optind][0] == '-') {
            switch (argv[optind][1]) {
            case 'c': {
                only_check = true;
                break;
            }
            case 'i': {
                do_inplace = true;
                break;
            }
            case 'n': {
                new_filename = argv[optind + 1];
                break;
            }
            case 'p': {
                pseudonym_metadata = argv[optind + 1];
                break;
            }
            case 'u': {
                disable_unlinking = true;
                break;
            }
            case 'm': {
                keep_macro_image = true;
                break;
            }
            case 'h': {
                print_help_message();
                exit(EXIT_FAILURE);
            }
            default: {
                fprintf(stderr, "Invalid arguments.\n");
                print_help_message();
                exit(EXIT_FAILURE);
            }
            }
        }
    }

    if (only_check) {
        if (filename != NULL) {
            int8_t format = check_file_format(filename);
            fprintf(stdout, "Vendor: [%s]\n", VENDOR_STRINGS[format]);
        } else {
            fprintf(stderr, "No filename to check for vendor selected.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        if (filename != NULL) {
            if (new_filename != NULL) {
                anonymize_wsi(filename, new_filename, pseudonym_metadata, keep_macro_image, disable_unlinking,
                              do_inplace);
            } else {
                // TODO: new file name (old_filename + tag)
                anonymize_wsi(filename, "_anonymized_wsi", pseudonym_metadata, keep_macro_image, disable_unlinking,
                              do_inplace);
            }
            fprintf(stdout, "Done.\n");
        } else {
            fprintf(stderr, "No file for anonymization selected.\n");
            exit(EXIT_FAILURE);
        }
    }
}