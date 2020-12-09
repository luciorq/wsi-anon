#ifndef HEADER_WSI_ANONYMIZER_H
#define HEADER_WSI_ANONYMIZER_H

#include "defines.h"

extern
file_format check_file_format(const char *filename);

extern
const char* anonymize_wsi(const char *filename, 
    const char *new_label_name,
    bool unlink_directory);

#endif