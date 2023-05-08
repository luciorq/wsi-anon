#include "b64.h"
#include "defines.h"
#include "jpec.h"
#include "utils.h"

char *wipe_section_of_attribute(char *buffer, char *attribute);

const char *get_value_from_attribute(char *buffer, char *attribute);

char *anonymize_value_of_attribute(char *buffer, char *attribute);

int32_t *get_height_and_width(const char *image_data);