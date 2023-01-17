#ifndef HEADER_TIFF_IO_H
#define HEADER_TIFF_IO_H

#include "defines.h"
#include "utils.h"
#include <inttypes.h>

static const char DOT_SVS[] = ".svs";
static const char SVS[] = "svs";
static const char TIF[] = "tif";
static const char DOT_TIF[] = ".tif";
static const char DOT_NDPI[] = ".ndpi";
static const char NDPI[] = "ndpi";
static const char BIF[] = "bif";
static const char DOT_BIF[] = ".bif";

#define min(a, b)                                                                                  \
    ({                                                                                             \
        __typeof__(a) _a = (a);                                                                    \
        __typeof__(b) _b = (b);                                                                    \
        _a < _b ? _a : _b;                                                                         \
    })

const char *duplicate_file(const char *filename, const char *new_label_name,
                           const char *file_extension);

int32_t handle_hamamatsu(const char **filename, const char *new_label_name, bool disable_unlinking,
                         bool do_inplace);

int32_t handle_aperio(const char **filename, const char *new_label_name, bool keep_macro_image,
                      bool disable_unlinking, bool do_inplace);

int32_t handle_ventana(const char **filename, const char *new_label_name, bool disable_unlinking,
                      bool do_inplace);

#endif
