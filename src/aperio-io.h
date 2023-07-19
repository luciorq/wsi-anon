#ifndef HEADER_APERIO_IO_H
#define HEADER_APERIO_IO_H

#include "tiff-based-io.h"

static const char DOT_SVS[] = ".svs";
static const char SVS[] = "svs";

// main functions
int32_t is_aperio(const char *filename);

int32_t handle_aperio(const char **filename, const char *new_filename, const char *pseudonym_metadata,
                      struct anon_configuration *configuration);

// additional functions
char *override_image_description(char *result, char *delimiter, const char *pseudonym);

struct metadata_attribute *get_attribute(const char *buffer, const char *delimiter1, const char *delimiter2);

struct metadata *get_metadata(file_t *fp, struct tiff_file *file);

struct wsi_data *get_wsi_data_aperio(const char *filename);

int32_t remove_metadata_in_aperio(file_t *fp, struct tiff_file *file, const char *pseudonym);

int32_t change_macro_image_compression_gt450(file_t *fp, struct tiff_file *file, int32_t directory);

#endif