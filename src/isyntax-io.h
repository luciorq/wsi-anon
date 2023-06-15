#ifndef HEADER_ISYNTAX_IO_H
#define HEADER_ISYNTAX_IO_H

#include "philips-based-io.h"

// main functions
int32_t is_isyntax(const char *filename);

int32_t handle_isyntax(const char **filename, const char *new_label_name, bool keep_macro_image, bool disable_unlinking,
                       bool do_inplace);

// additional functions
int32_t anonymize_isyntax_metadata(file_t *fp, int32_t header_size);

int32_t wipe_isyntax_image_data(file_t *fp, int32_t header_size, char *image_type);

#endif