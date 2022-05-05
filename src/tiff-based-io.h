#ifndef HEADER_TIFF_IO_H
#define HEADER_TIFF_IO_H

#include "defines.h"
#include "utils.h"

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

int32_t is_hamamatsu(const char *filename);

int32_t is_aperio(const char *filename);

int32_t is_ventana(const char *filename);

#endif
