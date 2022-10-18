/*
 *
 * Include this header file in a C-file and implement body for given functions in order to execute
 * anonymization on other file formats
 *
 */

// checks if the file format at hand is actually the expected file format
int32_t is_format(const char *filename);

// removes the label image
int32_t remove_label(file_t *fp);

// removes the macro image, if given
int32_t remove_macro(file_t *fp);

// removes all necessary metadata
int32_t remove_metadata(file_t *fp);

// executes the anonymization
int32_t handle_format(const char **filename, const char *new_label_name, bool disable_unlinking,
                      bool do_inplace);

/*
ToDo:
- include this header file in c files
- see if expandable approach like this works
- create tiff-based-io.h and tiff-based-io.c
- separate tiff based files
    - hamamatsu
    - aperio
    - ventana
- check for other data types in files that would not run on windows
- check how pass struct tiff_file *file to this function or reference it  in remove_metadata
function
- check if metadata in hamamatsu is actually removed
*/
