#include "defines.h"
#include "utils.h"

static const char ISYNTAX_EXT[] = "isyntax";
static const char DOT_ISYNTAX[] = ".isyntax";

int32_t get_size_to_substring(file_t *fp, char *substring);

int32_t file_contains_value(file_t *fp, char *value);

int32_t is_isyntax(const char *filename);

char *wipe_xml_data(char *result, char *attribute);

int32_t anonymize_isyntax_metadata(file_t *fp);

int32_t wipe_label_image(file_t *fp);

int32_t handle_isyntax(const char **filename, const char *new_label_name, bool keep_macro_image,
                       bool do_inplace);