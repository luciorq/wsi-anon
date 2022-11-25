#include "b64.h"
#include "defines.h"
#include "jpec.h"
#include "utils.h"

static const char ISYNTAX_EXT[] = "isyntax";
static const char DOT_ISYNTAX[] = ".isyntax";

int32_t get_size_to_substring(file_t *fp, char *substring);

int32_t file_contains_value(file_t *fp, char *value);

int32_t is_isyntax(const char *filename);

char *anonymize_value_of_attribute(char *buffer, char *attribute);

const char *get_value_from_attribute(char *buffer, char *attribute);

char *wipe_section_of_attribute(char *buffer, char *attribute);

int32_t anonymize_isyntax_metadata(file_t *fp, int32_t header_size);

int32_t *get_height_and_width(const char *image_data);

int32_t wipe_image_data(file_t *fp, int32_t header_size, char *image_type);

int32_t handle_isyntax(const char **filename, const char *new_label_name, bool keep_macro_image,
                       bool do_inplace);