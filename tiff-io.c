#include "tiffio.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <byteswap.h>

#define min(a,b) \
   ({ __typeof__ (a) _a = (a); \
       __typeof__ (b) _b = (b); \
     _a < _b ? _a : _b; })

struct tiff_entry {
    uint16_t tag;
    uint16_t type;
    uint64_t count;
    uint64_t offset;
    uint64_t start;
};

struct tiff_directory {
    uint64_t count;
    struct tiff_entry *entries;
    uint64_t in_pointer_offset;
    uint64_t out_pointer_offset;
    uint64_t number;
};

struct tiff_file {
    uint16_t count;
    struct tiff_directory *directories;
};

char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    // count number of elements we need to extract
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
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

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        // fill result array
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int file_exists(const char *filename)
{
    FILE *file;
    if ((file = fopen(filename, "r")))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

bool str_starts_with(const char *pre, const char *str)
{
    return strncmp(pre, str, strlen(pre)) == 0;
}

char *get_empty_char_buffer(char *x, uint64 times) {
    char *result = (char *)malloc(times * sizeof(char));
    if(times > 0) {
        // fill buffer with char *x
        for(int i = 0; i < times; i++) {
            result[i] = *x;
        }
    } else {
        result[0] = *x;
    }
    return result;
}

int blacken_label(TIFF *tiff, int unlink_dir) {
    uint32 width;
    uint32 height;
    TIFFGetField(tiff, TIFFTAG_IMAGEWIDTH, &width); 
    TIFFGetField(tiff, TIFFTAG_IMAGELENGTH, &height); 

    uint32* scan_line = (uint32*)_TIFFmalloc(width*(sizeof(uint32)));
    // create empty buffer with length of image width 
    // and pixel type uint32 (=4*char)
    char *charBuffer = get_empty_char_buffer("u", width*4);

    for(int i = 0; i < height; i++) {
        // coopy buffer content to allocated scanline buffer 
        // and write the line to the tiff file
        memcpy(scan_line, charBuffer, width*(sizeof(uint32)));
        TIFFWriteScanline(tiff, scan_line, i, 0);
    }

    // Unlink directory
    if(unlink_dir > 0) {
        TIFFUnlinkDirectory(tiff, TIFFCurrentDirectory(tiff) + 1);
    }

    return 1;
}

void fix_byte_order(void *data, int32_t size, int64_t count, bool big_endian) {
    switch(size) {
        case 1: break;
        case 2: {
            uint16_t *arr = data;
            for (int64_t i = 0; i < count; i++) {
                __bswap_16(arr[i]);
            }
            break;
        }
        case 4: {
            uint32_t *arr = data;
            for (int64_t i = 0; i < count; i++) {
                __bswap_32(arr[i]);
            }
            break;
        }
        case 8: {
            uint64_t *arr = data;
            for (int64_t i = 0; i < count; i++) {
                __bswap_64(arr[i]);
            }
            break;
        }
        default: break;
    }
}

uint64_t read_uint(FILE *fp, int32_t size, bool big_endian) {
    uint8_t buffer[size];
    if (fread(buffer, size, 1, fp) != 1) {
        return 0;
    }

    fix_byte_order(buffer, sizeof(buffer), 1, big_endian);

    switch (size) {
        case 1: {
            uint8_t result;
            memcpy(&result, buffer, sizeof(result));
            return result;
        }
        case 2: {
            uint16_t result;
            memcpy(&result, buffer, sizeof(result));
            return result;
        }
        case 4: {
            uint32_t result;
            memcpy(&result, buffer, sizeof(result));
            return result;
        }
        case 8: {
            uint64_t result;
            memcpy(&result, buffer, sizeof(result));
            return result;
        }
        default: {
            return 0;
        }
    }
}

uint32_t get_size_of_value(uint16_t type, uint64_t *count) {
    if(type == TIFF_BYTE || type == TIFF_ASCII || type == TIFF_SBYTE || type == TIFF_UNDEFINED) {
        return 1;
    } else if(type == TIFF_SHORT || type == TIFF_SSHORT) {
        return 2;
    } else if(type == TIFF_LONG || type == TIFF_SLONG || type == TIFF_FLOAT || type == TIFF_IFD) {
        return 4;
    } else if(type == TIFF_RATIONAL || type == TIFF_SRATIONAL) {
        *count *= 2;
        return 4;
    } else if(type == TIFF_DOUBLE || type == TIFF_LONG8 || type == TIFF_SLONG8 || type == TIFF_IFD8) {
        return 8;
    } else {
        return 0;
    }
}

uint64_t fix_ndpi_offset(uint64_t directory_offset, uint64_t offset) {
    uint64_t new_offset = (directory_offset & ~(uint64_t) UINT64_MAX) | (offset & UINT32_MAX);
    if(new_offset >= directory_offset) {
        new_offset = min(new_offset - UINT32_MAX - 1, new_offset);
    }
    return new_offset;
}

struct tiff_directory *read_tiff_directory(FILE *fp, 
        int64_t *dir_offset,
        struct tiff_directory *first_directory, 
        bool big_tiff, 
        bool ndpi, 
        bool big_endian,
        uint64_t *next_dir) {
    int64_t offset = *dir_offset;
    *dir_offset = 0;
    
    // seek to directory offset
    if(fseek(fp, offset, SEEK_SET) != 0) {
        printf("ERROR: seeking to offset failed\n");
    } 

    // read number of entries in directory
    uint64_t entry_count = read_uint(fp, big_tiff ? 8 : 2, big_endian);

    struct tiff_directory *tiff_dir = malloc(sizeof(struct tiff_directory));
    tiff_dir->count = entry_count;
    tiff_dir->in_pointer_offset = offset;

    struct tiff_entry *entries[entry_count];
    for (uint64_t i = 0; i < entry_count; i++) {
        struct tiff_entry *entry = malloc(sizeof(struct tiff_entry));

        if(entry == NULL) {
            printf("ERROR: could not allocate memory for entry\n");
        }

        entry->tag = read_uint(fp, 2, big_endian);
        entry->type = read_uint(fp, 2, big_endian);
        entry->count = read_uint(fp, big_tiff ? 8 : 4, big_endian);

        // calculate the size of the entry vaqlue
        uint32_t value_size = get_size_of_value(entry->type, &entry->count);

        if(!value_size) {
            printf("ERROR: calculating value size failed\n");
        }

        // read entry value to array
        uint8_t value[big_tiff ? 8 : 4];
        if (fread(value, sizeof(value), 1, fp) != 1) {
            printf("ERROR: reading value to array failed\n");
        }

        if(value_size * entry->count <= sizeof(value)) {
            // todo
        } else {
            if(big_tiff) {
                memcpy(&entry->offset, value, 8);
                fix_byte_order(&entry->offset, sizeof(entry->offset), 1, big_endian);
            } else {
                int32_t offset32;
                memcpy(&offset32, value, 4);
                fix_byte_order(&offset32, sizeof(offset32), 1, big_endian);
                entry->offset = offset32;
            }

            if(ndpi) {
                if(first_directory) {
                    // todo
                } else {
                    // todo
                }
            }
        }
        entries[i] = entry;
    }

    int64_t next_dir_offset = read_uint(fp, (big_tiff || ndpi) ? 8: 4, big_endian);

    tiff_dir->entries = *entries;
    tiff_dir->out_pointer_offset = next_dir_offset;
    next_dir = next_dir_offset;

    printf("next offset: %lu\n", next_dir_offset);

    return tiff_dir;
}

int check_hamamatsu_header(FILE *fp, bool *big_endian, bool *big_tiff) {
    int result = 0;
    uint16_t magic;

    // read endianness in header
    if(fread(&magic, sizeof magic, 1, fp) != 1) {
        return result;
    }
            
    if(magic == TIFF_BIGENDIAN || magic == TIFF_LITTLEENDIAN) {
        // check ndpi version in header
        *big_endian = (magic == TIFF_BIGENDIAN);
        uint16_t version = read_uint(fp, 2, *big_endian);

        if(version == TIFF_VERSION_BIG) {
            // try to read offset and padding for tiff version
            // if version is not classic
            uint16_t offset_size = read_uint(fp, 2, *big_endian);
            uint16_t pad = read_uint(fp, 2, *big_endian);
            if(offset_size == 8 && pad == 0) {
                *big_tiff = true;
                result = 1;
            }
        }  else if(version == TIFF_VERSION_CLASSIC) {
            *big_tiff = false;
            result = 1;
        }
    }
}

int handle_hamamatsu(const char *filename, const char *new_label_name) {
    FILE *fp;
    fp = fopen(filename, "r+w");

    // go to directory offset
    bool big_tiff = false;
    bool big_endian = false;
    int result = check_hamamatsu_header(fp, &big_endian, &big_tiff);

    if(result) {
        int64_t diroff = read_uint(fp, 8, big_endian);
        printf("Offset dir ponter: %ld\n", diroff);
        int64_t next_dir_offset = diroff;
        struct tiff_directory dir = *read_tiff_directory(fp, &diroff, NULL, big_tiff, true, big_endian, &next_dir_offset);

        while(next_dir_offset != 0) {

        }
    }
    
    fclose(fp);
    return 0;
}

int handle_aperio(const char *filename, const char *new_label_name) {
    TIFF* tiff = TIFFOpen(filename, "r+w");

    if(tiff) {
        int hasLabel = 0;
        int dircount = 0;
        do {
            char *description;
            TIFFGetField(tiff, TIFFTAG_IMAGEDESCRIPTION, &description);

            printf("%s\n", description);

            char** entries = str_split(description, '\n');
            size_t len = strlen(*entries);

            // correct directory is found when image description 
            // contains label tag
            if(len >= 2 && str_starts_with("label ",*(entries+1))) {
                hasLabel = 1;
                break;
            }
            dircount++;

            free(entries);
	    } while (TIFFReadDirectory(tiff));

        if(hasLabel == 1) {
            // set directory to label dir and blacken label
            TIFFSetDirectory(tiff, dircount);
            if(blacken_label(tiff, -1) == 1) {
                goto success;
            }
        } else {
            printf("No label found.");
        }
    }

    success: {
        TIFFClose(tiff);
        return 1;
    }

    TIFFClose(tiff);
    return 0;
}

int handle_mirax(const char *filename, const char *new_label_name) {
    // TODO
    return 0;
}

int is_hamamatsu(const char *filename) {
    int result = 0;
    const char *ext = get_filename_ext(filename);

    // check for valid file extension
    if(strcmp(ext, "ndpi") == 1) {
        return result;
    }

    FILE *fp = fopen(filename, "r");
    bool big_tiff = false;
    bool big_endian = false;
    result = check_hamamatsu_header(fp, &big_endian, &big_tiff);

    fclose(fp);
    return result;
}

int is_aperio(const char *filename) {
    int result = 0;
    const char *ext = get_filename_ext(filename);

    // check for valid file extension
    if(strcmp(ext, "svs") != 0 || strcmp(ext, "tif") != 0) {
        return result;
    }

    TIFF* tiff = TIFFOpen(filename, "r");

    if(tiff) {
        char *description;
        TIFFGetField(tiff, TIFFTAG_IMAGEDESCRIPTION, &description);

        // check for Aperio string in image description
        if(description != NULL  && strstr(description, "Aperio") != NULL) {
            result = 1;
        }
    }

    TIFFClose(tiff);

    return result;
}

int is_mirax(const char *filename) {
    const char *ext = get_filename_ext(filename);

    if(strcmp(ext, "mrsx") == 0) {
        return 1;
    } else {
        return 0;
    }  
}
