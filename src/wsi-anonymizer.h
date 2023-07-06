#ifndef HEADER_WSI_ANONYMIZER_H
#define HEADER_WSI_ANONYMIZER_H

#include "aperio-io.h"
#include "defines.h"
#include "hamamatsu-io.h"
#include "isyntax-io.h"
#include "mirax-io.h"
#include "philips-tiff-io.h"
#include "plugin.h"
#include "ventana-io.h"

static const char *VENDOR_STRINGS[] = {"Aperio",           "Hamamatsu",     "3DHistech (Mirax)", "Ventana",
                                       "Philips’ iSyntax", "Philips' TIFF", "Unknown",           "Invalid"};

extern int8_t check_file_format(const char *filename);

extern int32_t anonymize_wsi_inplace(const char *filename, const char *new_filename, const char *pseudonym_metadata,
                                     bool keep_macro_image, bool disable_unlinking);

extern int32_t anonymize_wsi(const char *filename, const char *new_filename, const char *pseudonym_metadata,
                             bool keep_macro_image, bool disable_unlinking, bool do_inplace);

#endif
