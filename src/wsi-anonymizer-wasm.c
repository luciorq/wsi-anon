#include "wsi-anonymizer.h"

#include <emscripten.h>

void EMSCRIPTEN_KEEPALIVE anonymize_wsi_inplace(const char *filename, const char *new_label_name, bool keep_macro_image, bool disbale_unlinking)
{
    anonymize_wsi(filename, new_label_name, keep_macro_image, disbale_unlinking, /* do_inplace = */ true);
}
