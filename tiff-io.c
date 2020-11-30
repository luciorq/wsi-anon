#include "tiffio.h"

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <byteswap.h>

#define NDPI_FORMAT_FLAG    65420
#define NDPI_SOURCELENS     65421

#define ASCII 2
#define SHORT 3
#define LONG 4
#define FLOAT 11
#define DOUBLE 12
#define LONG8 16

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
    uint64_t used;
    uint64_t size;
    struct tiff_directory *directories;
};

void init_tiff_file(struct tiff_file *file, size_t init_size) {
    file->directories = malloc(init_size * sizeof(struct tiff_directory));
    file->used = 0;
    file-> size = init_size;
}

void insert_dir_into_tiff_file(struct tiff_file *file, struct tiff_directory dir) {
    // reallocate directories array dynamically
    if(file->used == file->size) {
        file->size *= 2;
        file->directories = realloc(file->directories, file->size * sizeof(struct tiff_directory));
    }
    file->directories[file->used++] = dir;
}

void free_tiff_file(struct tiff_file *file) {
    free(file->directories);
    file->directories = NULL;
    file->used = file->size = 0;
}

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

char *get_empty_char_buffer(char *x, uint64 length) {
    char *result = (char *)malloc(length * sizeof(char));
    if(length > 0) {
        // fill buffer with char *x
        for(int i = 0; i < length; i++) {
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
        bool big_endian) {
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

    struct tiff_entry *entries = malloc(entry_count * sizeof(struct tiff_entry));
    for (uint64_t i = 0; i < entry_count; i++) {
        struct tiff_entry *entry = malloc(sizeof(struct tiff_entry));

        if(entry == NULL) {
            printf("ERROR: could not allocate memory for entry\n");
        }

        entry->start = ftell(fp);

        uint16_t tag = read_uint(fp, 2, big_endian);
        uint16_t type = read_uint(fp, 2, big_endian);
        uint64_t count = read_uint(fp, big_tiff ? 8 : 4, big_endian);
        entry->tag = tag;
        entry->type = type;
        entry->count = count;

        // calculate the size of the entry value
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
            fix_byte_order(value, value_size, count, big_endian);
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
                struct tiff_entry *_old_entry = NULL;
                if(first_directory) {
                    for (int j = 0; j < first_directory->count; j++) {
                        if(&first_directory->entries && first_directory->entries[j].tag == tag) {
                            _old_entry = &first_directory->entries[j];
                            break;
                        }
                    }
                } 
                
                if(!_old_entry || _old_entry->offset != entry->offset) {
                    entry->offset = fix_ndpi_offset(offset, entry->offset);
                }
            }
        }

        entries[i] = *entry;
    }

    int64_t next_dir_offset = read_uint(fp, (big_tiff || ndpi) ? 8: 4, big_endian);

    tiff_dir->entries = entries;
    tiff_dir->out_pointer_offset = next_dir_offset;
    *dir_offset =  next_dir_offset;

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
    return result;
}

struct tiff_file *read_tiff_file(FILE *fp, bool big_tiff, bool ndpi, bool big_endian) {
    // get directory offset; file stream pointer must be located just before the directory offset
    int64_t diroff = read_uint(fp, 8, big_endian);
    // reading the initial directory
    struct tiff_directory *dir = read_tiff_directory(fp, &diroff, NULL, big_tiff, ndpi, big_endian);

    // allocate memory for the tiff_file struct
    struct tiff_file *file = malloc(sizeof(struct tiff_file));
    // initialize tiff file with size 1
    init_tiff_file(file, 1);
    // insert the first directory
    insert_dir_into_tiff_file(file, *dir);
    // set the current directory as previous directory
    struct tiff_directory *prev_dir = dir;

    // when the directory offset is 0 we reached the end of the tiff file
    while(diroff != 0) {
        // read next directory
        struct tiff_directory *current_dir = read_tiff_directory(fp, &diroff, prev_dir, 
                                                    big_tiff, true, big_endian);
        // add directory to directory array and resize if necessary
        insert_dir_into_tiff_file(file, *current_dir);
        prev_dir = current_dir;
    }

    return file;
}


int get_label_dir(struct tiff_file *file, FILE *fp, bool big_endian) {
    for (int i = 0; i < file->used; i++) {
        struct tiff_directory temp_dir = file->directories[i];
        //printf("directory entry count: %lu\n", temp_dir.count);
            
        for(int j = 0; j < temp_dir.count; j++) {
            struct tiff_entry temp_entry = temp_dir.entries[j];
            //printf("entry tag: %u\n", temp_entry.tag);

            if(temp_entry.tag == NDPI_SOURCELENS) {
                int32_t entry_size = get_size_of_value(temp_entry.type, &temp_entry.count);

                if(entry_size) {

                    if(temp_entry.type == FLOAT) {
                        // move to function
                        float *v_buffer = malloc(entry_size * temp_entry.count);

                        // TODO make 8 byte generic!
                        uint64_t new_start = temp_entry.start + 8; 
                        if(fseek(fp, new_start, SEEK_SET)) {
                            printf("Failed to seek to offset %lu.\n", temp_entry.start);
                            return -1;
                        }
                        if(fread(v_buffer, entry_size, temp_entry.count, fp) != 1) {
                            printf("Failed to read entry value.\n");
                            return -1;
                        }

                        fix_byte_order(v_buffer, sizeof(float), 1, big_endian);

                        printf("Buffer: %f\n", *v_buffer);
                            
                        if(*v_buffer == -1) {
                            printf("delete label\n");  
                            return i;                       
                        }
                    }
                }
            }
        }
    }
    return -1;
}

uint64_t *read_pointer_by_tag(FILE *fp, struct tiff_directory *dir, int tag, bool big_endian) {
    int value;
    for(int i  = 0; i < dir->count; i++) {
        struct tiff_entry entry = dir->entries[i];
        if(entry.tag == tag) {
            int32_t entry_size = get_size_of_value(entry.type, &entry.count);
            printf("entry size: %i\n", entry_size);

            if(entry_size) {
                uint64_t *v_buffer = malloc(entry_size * entry.count);

                uint64_t new_start = entry.start + 8;
                printf("new start %lu\n", new_start);
                if(fseek(fp, new_start, SEEK_SET)) {
                    printf("Failed to seek to offset %lu.\n", entry.start);
                }
                if(fread(v_buffer, entry_size, entry.count, fp) != 1) {
                    printf("Failed to read entry value.\n");
                }

                fix_byte_order(v_buffer, sizeof(float), 1, big_endian);

                return *v_buffer;
            }
        }
    }
}

int delete_label(FILE *fp, struct tiff_directory *dir, bool big_endian) {
    // todo
    uint64_t *strip_offset = read_pointer_by_tag(fp, dir, TIFFTAG_STRIPOFFSETS, big_endian);
    uint64_t *strip_length = read_pointer_by_tag(fp, dir, TIFFTAG_STRIPBYTECOUNTS, big_endian);

    //printf("Strip offset: %i\n", strip_offset);
    //printf("Strip length: %i\n", strip_length);

    fseek(fp, strip_offset, SEEK_SET);

    //char *buf =  malloc(6);
    //fread(buf, 6, 1, fp);

    char *strip = get_empty_char_buffer("\0", strip_length);
    fwrite(strip, 1, strip_length, fp);
}

int handle_hamamatsu(const char *filename, const char *new_label_name) {
    FILE *fp;
    fp = fopen(filename, "r+w");

    // go to directory offset
    bool big_tiff = false;
    bool big_endian = false;
    int result = check_hamamatsu_header(fp, &big_endian, &big_tiff);

    if(result) {
        struct tiff_file *file;
        file = read_tiff_file(fp, big_tiff, true, big_endian);

        int dir_count = get_label_dir(file, fp, big_endian);

        if(dir_count == -1) {
            printf("Error: No label directory.");
        }

        struct tiff_directory dir = file->directories[dir_count];

        printf("Delete: %lu\n", dir.out_pointer_offset);

        // wipe label data from directory
        delete_label(fp, &dir, big_endian);

        // unlink directory and link predecessor to last dir
        uint64_t out_pointer = dir.out_pointer_offset;


        free_tiff_file(file);
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