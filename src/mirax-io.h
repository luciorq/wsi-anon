#ifndef HEADER_MIRAX_IO_H
#define HEADER_MIRAX_IO_H

#include "defines.h"
#include "tiff-based-io.h"
#include "ini-parser.h"
#include "utils.h"

int handle_mirax(char *filename, 
    const char *new_label_name, 
    bool disbale_unlinking,
    bool disable_inplace);

int is_mirax(const char *filename);

#endif
