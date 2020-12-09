#include "utils.h"

// split a string by a given delimiter
char** str_split(char* a_str, const char a_delim) {
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    // count the number of elements we need to extract
    while(*tmp) {
        if (a_delim == *tmp) {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    // add space for trailing token
    count += last_comma < (a_str + strlen(a_str) - 1);

    // add space for terminating null string so caller
    // knows where the list of returned strings ends
    count++;

    result = (char **)malloc(sizeof(char*) * count);

    if(result) {
        // fill result array
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while(token) {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

// get the filename extension for a given filename
const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int32_t file_exists(const char *filename) {
    FILE *file;
    if ((file = fopen(filename, "r"))) {
        fclose(file);
        return 1;
    }
    return 0;
}

// return a char buffer filled with char x
char *get_empty_char_buffer(const char *x, uint64_t length) {
    char *result = (char *)malloc(length * sizeof(char));
    if(length > 0) {
        // fill buffer with char *x
        for(uint64_t i = 0; i < length; i++) {
            result[i] = *x;
        }
    } else {
        result[0] = *x;
    }
    return result;
}

bool starts_with(char *str, const char *pre) {
    return strlen(str) < strlen(pre) ? false : memcmp(pre, str, strlen(pre)) == 0;
}

char *get_string_between_delimiters(char *buffer, const char *delimiter1, const char *delimiter2) {
    const char *substring1 = strstr(buffer, delimiter1);
    if(substring1) {
        const size_t s_del1 = strlen(delimiter1);
        if(delimiter2) {
            const char *substring2 = strstr(substring1 + s_del1, delimiter2);
            const size_t mlen = substring2 - (substring1 + s_del1);
            char *result = (char *)malloc(mlen + 1);
            if(result)
            {
                memcpy(result, substring1 + s_del1, mlen);
                result[mlen] = '\0';
                return result;
            }
        }
    }
    return NULL;
}

void remove_leading_spaces(char *str) {
    char * p = str;
    int l = strlen(p);

    while(isspace(p[l - 1])) p[--l] = 0;
    while(* p && isspace(* p)) ++p, --l;

    memmove(str, p, l + 1);
}

char *concat_path_filename(const char *path, const char *filename) {
    char *new_string = (char *)malloc(strlen(path) + strlen(filename) + 3);
    strcpy(new_string, path);
    strcat(new_string, "//");
    strcat(new_string, filename);
    return new_string;
}

char *int32_to_str(int32_t integer) {
    int length = snprintf( NULL, 0, "%d", integer);
    char* str = (char *)malloc(length + 1);
    snprintf( str, length + 1, "%d", integer);
    return str;
}

int32_t number_of_digits(int32_t integer) {
    int32_t result = (integer < 0) ? 1 : 0;
    while((integer /= 10) != 0) {
        result++;
    }
    return result;
}

char *add_square_brackets(char *str) {
    int32_t length = sizeof(str) + sizeof("[") + sizeof("]") + sizeof("\0") + 1;
    char *result = (char *)malloc(length);
    strcpy(result, "[");
    strcat(result, str);
    strcat(result, "]");
    strcat(result, "\0");
    free(str);
    return result;
}

char *add_equals_sign(char *str1, char *str2) {
    int32_t length = sizeof(str1) + sizeof(str2) + sizeof(" = ") + sizeof("\0") + 1;
    char *result = (char *)malloc(length);
    strcpy(result, str1);
    strcat(result, " = ");
    strcat(result, str2);
    strcat(result, "\0");
    free(str1);
    free(str2);
    return result;
}

// determine wether the operating system is big or little endian
bool is_system_big_endian() {
  int n = 1;
  if(*(char *)&n == 1) {
    return true;
  }
  return false;
}

// swap bytes of unsigned interger 16 bytes
uint16_t _swap_uint16(uint16_t value) {
    return (value << 8) | (value >> 8 );
}

// swap bytes of unsigned interger 32 bytes
uint32_t _swap_uint32(uint32_t value) {
    value = ((value << 8) & 0xFF00FF00 ) | ((value >> 8) & 0xFF00FF ); 
    return (value << 16) | (value >> 16);
}

// swap bytes of unsigned interger 32 bytes
uint64_t _swap_uint64(uint64_t value) {
    value = ((value << 8) & 0xFF00FF00FF00FF00ULL ) | ((value >> 8) & 0x00FF00FF00FF00FFULL );
    value = ((value << 16) & 0xFFFF0000FFFF0000ULL ) | ((value >> 16) & 0x0000FFFF0000FFFFULL );
    return (value << 32) | (value >> 32);
}