#ifndef HEADER_UTILS_H
#define HEADER_UTILS_H

#include "defines.h"

// string operations
char** str_split(char* a_str, const char a_delim);
const char *get_filename_ext(const char *filename);
int32_t file_exists(const char *filename);
char *get_empty_char_buffer(const char *x, 
    uint64_t length, 
    const char *prefix);
bool starts_with(char *str, const char *pre);
char *get_string_between_delimiters(char *buffer, 
    const char *delimiter1, 
    const char *delimiter2);
char *string_remove_char_from_charset(char *string, char *spanset);
void remove_leading_spaces(char *str);
char *concat_path_filename(const char *path, const char *filename);
char *int32_to_str(int32_t integer);
int32_t number_of_digits(int32_t integer);
char *add_square_brackets(char *str);
char *add_equals_sign(char *str1, char *str2);
bool contains(const char *str1, const char *str2);

// byte operations
bool is_system_big_endian();
uint16_t _swap_uint16(uint16_t value);
uint32_t _swap_uint32(uint32_t value);
uint64_t _swap_uint64(uint64_t value);

#endif