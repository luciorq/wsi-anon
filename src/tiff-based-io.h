#ifndef HEADER_TIFF_IO_H
#define HEADER_TIFF_IO_H

#include "tiff-based-io.h"
#include "mirax-io.h"
#include "defines.h"
#include "utils.h"

// libtiff
#include "tiffio.h"

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

int handle_hamamatsu(const char *filename, const char *new_label_name, bool unlink_directory);
int handle_aperio(const char *filename, const char *new_label_name, bool unlink_directory);

int is_hamamatsu(const char *filename);
int is_aperio(const char *filename);

int file_exists(const char *filename);
const char *get_filename_ext(const char *filename);

#endif