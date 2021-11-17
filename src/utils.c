#include "utils.h"

// split a string by a given delimiter
char** str_split(char* a_str, const char a_delim) {
    char **result    = 0;
    size_t count     = 0;
    char *tmp        = a_str;
    char *last_comma = 0;
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

    result = (char **)malloc(sizeof(char *) *count);

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
    file_t *file;
    if ((file = file_open(filename, "r"))) {
        file_close(file);
        return 1;
    }
    return 0;
}

// return a char buffer filled with char x
char *get_empty_char_buffer(const char *x, 
        uint64_t length, 
        const char *prefix) {
    // assure length is a postive integer and not zero
    if(length < 1) {
        fprintf(stderr, "Error: Length smaller 1.\n");
        return NULL;
    }

    // allocate buffer
    char *result = (char *)malloc((length * sizeof(char)) + 1);

    int32_t start = 0;
    if(prefix != NULL) {
        int32_t str_length = strlen(prefix);
        // add prefix before empty buffer (e.g. JPEG SOI)
        if(length > str_length) {
            for(; start < str_length; start++) {
                result[start] = prefix[start];
            }
        }
    }

    // fill rest of buffer with x
    for(int32_t i = start; i <= length; i++) {
            result[i] = *x;
    }

    return result;
}

bool starts_with(char *str, const char *pre) {
    return strlen(str) < strlen(pre) ? false : 
        memcmp(pre, str, strlen(pre)) == 0;
}

char *get_string_between_delimiters(char *buffer, 
        const char *delimiter1, 
        const char *delimiter2) {
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

char *concat_path_filename(const char *path, 
        const char *filename) {
    char *new_string = (char *)malloc(strlen(path) 
                            + strlen(filename) + 3);
    strcpy(new_string, path);
    strcat(new_string, "/");
    strcat(new_string, filename);
    return new_string;
}

char *concat_path_filename_ext(const char *path, 
        const char *filename,
        const char *ext) {
    char *new_string = (char *)malloc(strlen(path) 
                            + strlen(filename) + strlen(ext) + 2);
    strcpy(new_string, path);
    // we assume that path is already ending with a dash
    strcat(new_string, filename);
    //strcat(new_string, ".");
    strcat(new_string, ext);
    return new_string;
}

char *get_filename_from_path(char *path)
{
    if(path == NULL )
        return NULL;

    char *temp_path = path;
    for(char *p_cur = path; *p_cur != '\0'; p_cur++)
    {
        if(*p_cur == '/' || *p_cur == '\\')
            temp_path = p_cur+1;
    }
    
    return temp_path;
}

char *int32_to_str(int32_t integer) {
    int length = snprintf( NULL, 0, "%d", integer);
    char* str = (char *)malloc(length + 1);
    snprintf( str, length + 1, "%d", integer);
    return str;
}

int32_t number_of_digits(int32_t integer) {
    int32_t result = (integer < 0) ? 2 : 1;
    while((integer /= 10) != 0) {
        result++;
    }
    return result;
}

// put a given string in square brackets
char *add_square_brackets(const char *str) {
    int32_t length = strlen(str) + sizeof("[") 
                        + sizeof("]") + sizeof("\0") + 1;
    char *result = (char *)malloc(length);
    strcpy(result, "[");
    strcat(result, str);
    strcat(result, "]");
    strcat(result, "\0");
    return result;
}

// add an equal sign with whitespaces between to strings
char *add_equals_sign(const char *str1, const char *str2) {
    int32_t length = strlen(str1) + strlen(str2) 
                        + sizeof(" = ") + sizeof("\0") + 1;
    char *result = (char *)malloc(length);
    strcpy(result, str1);
    strcat(result, " = ");
    strcat(result, str2);
    strcat(result, "\0");
    return result;
}

// function to check if str1 contains str2
bool contains(const char *str1, const char *str2) {
    int i = 0, j = 0;

    while(str1[i] != '\0') {
        if(str1[i] == str2[j]) {
            while(str1[i] == str2[j] && str2[j] != '\0') {
                j++;
                i++;
            }
            if(str2[j] == '\0') {
                return true;
            }
            j = 0;
        }
        i++;
    }
    return false;
}

int32_t copy_file_v2(const char *src, const char *dest) {
    char command[strlen(src) + strlen(dest) + 15];    
#ifdef __linux__ 
    // we create the copy command for linux
    snprintf(command, sizeof command, "cp \"%s\" \"%s\"%c", src, dest, '\0');
    return system(command);
#elif _WIN32
    // we create the copy command for win
    snprintf(command, sizeof command, "xcopy \"%s\" \"%s\"%c", src, dest, '\0');
    return system(command);
#else
    // todo: implement for mac
    return -1;
#endif
}

int32_t copy_file(const char *src, const char *dest) {
    file_t *source, *target;
 
    source = file_open(src, "r");
 
    if(source == NULL) {
        fprintf(stderr, "Could not open source file.\n");
        return -1;
    }
 
    target = file_open(dest, "w");
 
    if(target == NULL) {
       file_close(source);
       fprintf(stderr, "Could not open destination file.\n");
       return -1;
    }

    char ch;
    while((ch = file_getc(source)) != EOF) {
        file_putc(ch, target);
    }

    file_close(source);
    file_close(target);
 
    return 0;
}

int32_t copy_directory(const char *src , const char *dest) {
    char command[strlen(src) + strlen(dest) + 15];    
#ifdef __linux__ 
    // we create the copy command for linux
    snprintf(command, sizeof command, "cp -r \"%s\" \"%s\"%c", src, dest, '\0');
    return system(command);
#elif _WIN32
    // we create the copy command for win
    snprintf(command, sizeof command, "xcopy \"%s\" \"%s\" /s /e%c", src, dest, '\0');
    return system(command);
#else
    // todo: implement for mac
    return -1;
#endif
}

// determine wether the operating system 
// is big or little endian
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
    value = ((value << 8) & 0xFF00FF00FF00FF00ULL ) 
                | ((value >> 8) & 0x00FF00FF00FF00FFULL );
    value = ((value << 16) & 0xFFFF0000FFFF0000ULL ) 
                | ((value >> 16) & 0x0000FFFF0000FFFFULL );
    return (value << 32) | (value >> 32);
}