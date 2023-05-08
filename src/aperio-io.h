#include "tiff-based-io.h"

static const char DOT_SVS[] = ".svs";
static const char SVS[] = "svs";

// main functions
int32_t is_aperio(const char *filename);

int32_t handle_aperio(const char **filename, const char *new_label_name, bool keep_macro_image,
                      bool disable_unlinking, bool do_inplace);

// additional functions
char *override_image_description(char *result, char *delimiter);

int32_t remove_metadata_in_aperio(file_t *fp, struct tiff_file *file);

int32_t change_macro_image_compression_gt450(file_t *fp, struct tiff_file *file, int32_t directory);