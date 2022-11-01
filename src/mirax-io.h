#ifndef HEADER_MIRAX_IO_H
#define HEADER_MIRAX_IO_H

#include "defines.h"
#include "ini-parser.h"
#include "tiff-based-io.h"
#include "utils.h"

int32_t handle_mirax(const char **filename, const char *new_label_name, bool keep_macro_image,
                     bool disbale_unlinking, bool do_inplace);

int32_t is_mirax(const char *filename);

#endif
