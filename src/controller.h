/*
 *
 * Include this header file in a C-file and implement body for given functions in order to execute
 * anonymization on other file formats
 *
 * ToDo:
 * - add your format to VENDOR_STRINGS array in console-app.c 
 * - add your format to enum file_format in defines.h
 * - implement the following functions describes as below
 *
 */

#include <stdint.h>
#include <stdbool.h>

// checks if the file format at hand is actually the expected file format
int32_t is_format(const char *filename);

// implements the anonymization that is called within the handle_<YOUR_FORMAT>() function
int32_t handle_format(const char **filename, const char *new_label_name,
                                  bool keep_macro_image, bool disable_unlinking, bool do_inplace);

/*
ToDo:
- separate tiff based files
    - hamamatsu
    - aperio
    - ventana
- check for other data types in files that would not run on windows
- check if metadata in hamamatsu is actually removed
--------------test this out------------

- refactor check_file_header() 
--> to return height and width of image
--> metadata (how?? what??)


--> Header file does not seem to work, try out macros

*/
