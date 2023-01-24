#ifndef HEADER_WSI_ANONYMIZER_H
#define HEADER_WSI_ANONYMIZER_H

#include "defines.h"
#include "isyntax-io.h"
#include "mirax-io.h"
#include "ventana-io.h"
#include "hamamatsu-io.h"
#include "aperio-io.h"

extern int8_t check_file_format(const char *filename);

extern int32_t anonymize_wsi_inplace(const char *filename, const char *new_label_name,
                                     bool keep_macro_image, bool disable_unlinking);

extern const char *anonymize_wsi(const char *filename, const char *new_label_name,
                                 bool keep_macro_image, bool disable_unlinking, bool do_inplace);

extern void freeMem(void *ptr);

#endif
