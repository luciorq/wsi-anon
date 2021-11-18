#ifndef HEADER_WSI_ANONYMIZER_H
#define HEADER_WSI_ANONYMIZER_H

#include "defines.h"
#include "tiff-based-io.h"
#include "mirax-io.h"

extern
file_format check_file_format(const char *filename);

extern
const char* anonymize_wsi(const char *filename, 
    const char *new_label_name,
    bool keep_macro_image,
    bool unlink_directory,
    bool do_inplace);

#endif