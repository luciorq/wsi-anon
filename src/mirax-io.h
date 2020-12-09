#ifndef HEADER_MIRAX_IO_H
#define HEADER_MIRAX_IO_H

#include "defines.h"
#include "tiff-based-io.h"
#include "ini-parser.h"
#include "utils.h"

int handle_mirax(const char *filename, const char *new_label_name, bool unlink_directory);

int is_mirax(const char *filename);

#endif
