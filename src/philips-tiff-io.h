#ifndef HEADER_PHILIPS_IO_H
#define HEADER_PHILIPS_IO_H

#include "philips-based-io.h"
#include "tiff-based-io.h"

static const char DOT_TIFF[] = ".tiff";
static const char TIFF[] = "tiff";

// main functions
int32_t is_philips_tiff(const char *filename);

int32_t handle_philips_tiff(const char **filename, const char *new_filename, const char *pseudonym_metadata,
                            struct anon_configuration *configuration);

// additional functions
int32_t wipe_philips_image_data(file_t *fp, struct tiff_file *file, char *image_type);

int32_t anonymize_philips_metadata(file_t *fp, struct tiff_file *file, const char *pseudonym);

#endif