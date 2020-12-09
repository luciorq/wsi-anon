#ifndef HEADER_INI_PARSER_H
#define HEADER_INI_PARSER_H

#include "defines.h"
#include "utils.h"

struct ini_file *read_slidedat_ini_file(const char *path, 
        const char *ini_filename);

char *get_value_from_ini_file(struct ini_file *ini_file, 
        const char *group, 
        const char *entry_key);

int32_t get_all_records(struct ini_file *ini_file);

int32_t delete_group_form_ini_file(struct ini_file *ini_file, 
        char *group_name);

void rename_section_name_for_level_in_section(struct ini_file *ini_file, 
        const char *group_name, 
        struct mirax_level *current_level, 
        struct mirax_level *next_level);

void set_value_for_group_and_key(struct ini_file *ini_file, 
        const char *group_name, 
        char *key, 
        char *value);

void remove_entry_for_group_and_key(struct ini_file *ini_file, 
        const char *group_name, 
        const char *key);

void decrement_value_for_group_and_key(struct ini_file *ini_file, 
        const char *group_name, 
        const char *key);

int32_t write_ini_file(struct ini_file *ini_file, 
        const char *path, 
        const char *filename);

#endif