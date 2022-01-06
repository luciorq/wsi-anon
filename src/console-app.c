#include "wsi-anonymizer.h"

static const char *VENDOR_STRINGS[] = {"Aperio", "Hamamatsu", "3DHistech (Mirax)", "Unknown",
                                       "Invalid"};

void print_help_message() {
    fprintf(stderr, "Usage: ./wsi-anon [FILE] [-OPTIONS]\n\n");
    fprintf(stderr, "OPTIONS:\n");
    fprintf(stderr, "-c     only check file for vendor format\n");
    fprintf(stderr, "-n     specify new label name (e.g. -n \"labelname\")\n");
    fprintf(stderr, "-m     if flag is set, macro image will NOT be deleted\n");
    fprintf(stderr, "-i     if flag is set, anonymization will be done in-place\n");
    fprintf(stderr, "-u     if flag is set, tiff directory will NOT be unlinked\n\n");
}

int main(int argc, char *argv[]) {
    bool only_check = false;
    bool keep_macro_image = false;
    bool disable_unlinking = false;
    bool do_inplace = false;
    const char *filename = NULL;
    const char *new_label_name = NULL;

    if (argv[1] == NULL) {
        fprintf(stderr, "No filename specified.\n");
        exit(EXIT_FAILURE);
    }

    filename = argv[1];

    if (strcmp(filename, "-h\0") == 0) {
        print_help_message();
        exit(EXIT_FAILURE);
    }

    size_t optind;
    for (optind = 2; optind < argc; optind++) {
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
                new_label_name = argv[optind + 1];
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
            file_format format = check_file_format(filename);
            fprintf(stdout, "Vendor: [%s]\n", VENDOR_STRINGS[format]);
        } else {
            fprintf(stderr, "No filename to check for vendor selected.\n");
            exit(EXIT_FAILURE);
        }
    } else {
        if (filename != NULL) {
            if (new_label_name != NULL) {
                anonymize_wsi(filename, new_label_name, keep_macro_image, disable_unlinking,
                              do_inplace);
            } else {
                // TODO: new file name (old_filename + tag)
                anonymize_wsi(filename, "_anonymized_wsi", keep_macro_image, disable_unlinking,
                              do_inplace);
            }
            fprintf(stdout, "Done.\n");
        } else {
            fprintf(stderr, "No file for anonymization selected.\n");
            exit(EXIT_FAILURE);
        }
    }
}