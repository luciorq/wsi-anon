#include "b64.h"
#include "defines.h"
#include "utils.h"

static const char ISYNTAX_EXT[] = "isyntax";
static const char DOT_ISYNTAX[] = ".isyntax";

int32_t get_size_to_substring(file_t *fp, char *substring);

int32_t file_contains_value(file_t *fp, char *value);

int32_t is_isyntax(const char *filename);

char *wipe_xml_data(char *result, char *attribute);

int32_t anonymize_isyntax_metadata(file_t *fp);

int32_t wipe_image_data(file_t *fp, char *image_type);

int32_t handle_isyntax(const char **filename, const char *new_label_name, bool keep_macro_image,
                       bool do_inplace);

// --- REFACTOR FROM HERE, ADD TO NEW FILE ----
void create_black_image(unsigned char *new_image, unsigned char *width_str,
                        unsigned char *height_str);

static void add_soi(unsigned char *new_image);

static void add_app(unsigned char *new_image);

static void add_eoi(unsigned char *new_image);

static void add_segment(unsigned char *new_image, unsigned char *data);