#ifndef HEADER_ISYNTAX_IO_H
#define HEADER_ISYNTAX_IO_H

#include "philips-based-io.h"

// main functions
int32_t is_isyntax(const char *filename);

int32_t handle_isyntax(const char **filename, const char *new_filename, const char *pseudonym_metadata,
                       struct anon_configuration configuration);

// additional functions
int32_t anonymize_isyntax_metadata(file_t *fp, int32_t header_size, const char *pseudonym);

int32_t wipe_isyntax_image_data(file_t *fp, size_t header_size, char *image_type);

#endif