#include "tiff-based-io.h"

static const char BIF[] = "bif";
static const char DOT_BIF[] = ".bif";

// main functions
int32_t is_ventana(const char *filename);

int32_t handle_ventana(const char **filename, const char *new_label_name, bool keep_macro_image, bool disable_unlinking,
                       bool do_inplace);

// additional functions
int64_t get_ventana_label_dir(file_t *fp, struct tiff_file *file);

int32_t wipe_label_ventana(file_t *fp, struct tiff_directory *dir, bool big_endian);

int32_t wipe_and_unlink_ventana_directory(file_t *fp, struct tiff_file *file, int64_t directory, bool big_endian,
                                          bool disable_unlinking);

char *wipe_xmp_data(char *result, char *delimiter1, char *delimiter2);

int32_t remove_metadata_in_ventana(file_t *fp, struct tiff_file *file);