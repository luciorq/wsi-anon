#include "controller.h"
#include "tiff-based-io.h"

// main functions 
int32_t is_hamamatsu(const char *filename);

int32_t handle_hamamatsu(const char **filename, const char *new_label_name, bool disable_unlinking, bool do_inplace);

// additinonal functions 
int32_t get_hamamatsu_macro_dir(struct tiff_file *file, file_t *fp, bool big_endian);