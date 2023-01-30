/*
 *
 * Include this header file in your source file in order to execute anonymization on an additional
 * format. Add your format to the 'VENDOR_STRINGS' variable in wsi-anonymizer.h right before the
 * 'Unknown' entry. Implement the following functions declared below. Add is_format() and
 * handle_format() to the *is_format_functions[] and *handle_format_functions[] arrays in
 * wsi-anonymizer.c.
 *
 */

#include <stdbool.h>
#include <stdint.h>

// checks if the file format at hand is actually the expected file format
int32_t is_format(const char *filename);

// implements the anonymization that is called within the handle_<YOUR_FORMAT>() function
int32_t handle_format(const char **filename, const char *new_label_name, bool keep_macro_image,
                      bool disable_unlinking, bool do_inplace);
