#ifndef HEADER_HAMAMATSU_H
#define HEADER_HAMAMATSU_H

#include "tiff-based-io.h"

static const char DOT_NDPI[] = ".ndpi";
static const char NDPI[] = "ndpi";

// main functions
int32_t is_hamamatsu(const char *filename);

int32_t handle_hamamatsu(const char **filename, const char *new_label_name, bool keep_macro_image,
                         bool disable_unlinking, bool do_inplace);

// additinonal functions
int32_t get_hamamatsu_macro_dir(struct tiff_file *file, file_t *fp, bool big_endian);

#endif