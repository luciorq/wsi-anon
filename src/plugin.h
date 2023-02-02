/*
 *
 * In order to execute anonymization on an additional format, include this header file in the header
 * file of your new format. For conventional purposes name the newly created header and source file
 * of your format <YOUR_FORMAT>-io.h and <YOUR_FORMAT>-io.c. Implement the functions declared below.
 * You can either keep these functionnames or use them as a template and change them to
 * is_<YOUR_FORMAT>() and handle_<YOUR_FORMAT>() (this is recommenended when you want to implement
 * the anonymiaztion of more than one format). Add your format to the 'VENDOR_STRINGS' variable in
 * wsi-anonymizer.h right before the 'Unknown' entry. Lastly, add &is_format() and &handle_format()
 * (or &is_<YOUR_FORMAT>() and &handle_<YOUR_FORMAT>()) to the *is_format_functions[] and
 * *handle_format_functions[] arrays in wsi-anonymizer.c.
 *
 */

#include <stdbool.h>
#include <stdint.h>

// checks if the file format at hand is actually the expected file format
int32_t is_format(const char *filename);

// implements the anonymization for the added format
int32_t handle_format(const char **filename, const char *new_label_name, bool keep_macro_image,
                      bool disable_unlinking, bool do_inplace);