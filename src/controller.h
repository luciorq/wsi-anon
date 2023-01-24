/*
 *
 * Include this header file in a C-file and implement body for given functions in order to execute
 * anonymization on other file formats
 *
 * ToDo:
 * - add your format to VENDOR_STRINGS array in console-app.c 
 * - add your format to enum file_format in defines.h
 * - implement the following functions as described as below
 * - declare all additional functions in a new header file with the same name as your source file
 *
 */

#include <stdint.h>
#include <stdbool.h>

// checks if the file format at hand is actually the expected file format
inline int32_t is_format(const char *filename);

// implements the anonymization that is called within the handle_<YOUR_FORMAT>() function
inline int32_t handle_format(const char **filename, const char *new_label_name, bool keep_macro_image, bool disable_unlinking, bool do_inplace);

/*
ToDo:
- separate tiff based files
    - hamamatsu
    - aperio
    - ventana
- check for other data types in files that would not run on windows
- check if metadata in hamamatsu is actually removed
--------------test this out------------

- see link for self inclusion + macros for dynamically calling a function 
- https://stackoverflow.com/questions/22701342/c-dynamic-function-call
- int (*array[])(void) = {test1, test2, test3}; 
    - test1, test2, test3 sind Funktionen
*/
