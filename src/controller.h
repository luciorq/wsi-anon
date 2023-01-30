/*
 *
 * Include this header file in your source file named <YOUR_FORMAT>-io.c and implement
 * the given functions in order to execute anonymization
 *
 * ToDo:
 * - add your format to the VENDOR_STRINGS array in wsi-anonymizer.h right before the 'Unknown'
 entry
 * - implement the following functions as described below (see source files of other formats)
 * - add is_<YOUR_FORMAT>() and handle_<YOUR_FORMAT>() to *is_format_functions[] and
 *handle_format_functions[] function arrays in wsi-anonymizer.c
 * - declare all additional functions in a new header file with the same name as your source file
 *
 */

#include <stdbool.h>
#include <stdint.h>

// checks if the file format at hand is actually the expected file format
inline int32_t is_format(const char *filename);

// implements the anonymization that is called within the handle_<YOUR_FORMAT>() function
inline int32_t handle_format(const char **filename, const char *new_label_name,
                             bool keep_macro_image, bool disable_unlinking, bool do_inplace);

/*
ToDo:
- check for both windows and ubuntu
- remove warnings when compiling
*/
