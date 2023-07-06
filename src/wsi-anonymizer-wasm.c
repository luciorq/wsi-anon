#include "wsi-anonymizer.h"

#include <emscripten.h>

int32_t EMSCRIPTEN_KEEPALIVE wsi_anonymize(const char *filename, const char *new_filename,
                                           const char *pseudonym_metadata, bool keep_macro_image,
                                           bool disbale_unlinking) {
    return anonymize_wsi_inplace(filename, new_filename, pseudonym_metadata, keep_macro_image, disbale_unlinking);
}
