#ifndef HEADER_UTILS_H
#define HEADER_UTILS_H

#include "defines.h"
#include "file-api.h"

// string operations
char **str_split(char *a_str, const char a_delim);

const char *get_filename_ext(const char *filename);

int32_t file_exists(const char *filename);

char *get_empty_char_buffer(const char *x, uint64_t length, const char *prefix, const char *suffix);

char *anonymize_string(const char *x, uint64_t length);

char *replace_str(const char *original_str, const char *replace_str, const char *with_str);

bool starts_with(const char *str, const char *pre);

const char *get_string_between_delimiters(const char *buffer, const char *delimiter1,
                                          const char *delimiter2);

void remove_leading_spaces(char *str);

const char *concat_str(const char *str1, const char *str2);

const char *concat_path_filename(const char *path, const char *filename);

const char *concat_path_filename_ext(const char *path, const char *filename, const char *ext);

const char *get_filename_from_path(const char *path);

const char *int32_to_str(int32_t integer);

int32_t number_of_digits(int32_t integer);

const char *add_square_brackets(const char *str);

const char *add_equals_sign(const char *str1, const char *str2);

bool contains(const char *str1, const char *str2);

// file operations
const char *duplicate_file(const char *filename, const char *new_file_name,
                           const char *file_extension);

int32_t copy_file_v2(const char *src, const char *dest);

int32_t copy_file(const char *src, const char *dest);

int32_t copy_directory(const char *src, const char *dest);

// byte operations
bool is_system_big_endian();

uint16_t _swap_uint16(uint16_t value);

uint32_t _swap_uint32(uint32_t value);

uint64_t _swap_uint64(uint64_t value);

const char *skip_first_and_last_char(const char *value);

int count_contains(const char *str1, const char *str2);

#endif