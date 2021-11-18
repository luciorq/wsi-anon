#ifndef HEADER_DEFINES_H
#define HEADER_DEFINES_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>

#define MAX_CHAR_IN_LINE 100

// mirax
#define MRXS_ROOT_OFFSET_NONHIER 41

// tiff like
#define TIFF_BIGENDIAN      0x4d4d
#define TIFF_LITTLEENDIAN   0x4949

#define TIFF_VERSION_CLASSIC        42
#define TIFF_VERSION_BIG            43
#define	TIFFTAG_IMAGEDESCRIPTION    270	
#define	TIFFTAG_STRIPOFFSETS        273
#define TIFFTAG_STRIPBYTECOUNTS     279

typedef enum {
	TIFF_NOTYPE = 0, 
	TIFF_BYTE = 1, 
	TIFF_ASCII = 2,
	TIFF_SHORT = 3,
    TIFF_LONG = 4,
	TIFF_RATIONAL = 5,
	TIFF_SBYTE = 6,
	TIFF_UNDEFINED = 7,
	TIFF_SSHORT = 8,
	TIFF_SLONG = 9,
	TIFF_SRATIONAL = 10,
	TIFF_FLOAT = 11,
	TIFF_DOUBLE = 12,
	TIFF_IFD = 13,
	TIFF_LONG8 = 16,
	TIFF_SLONG8 = 17,
	TIFF_IFD8 = 18
} TIFFDataType;

#define ASCII 2
#define SHORT 3
#define LONG 4
#define FLOAT 11
#define DOUBLE 12
#define LONG8 16

#define JPEG_SOI "\xff\xd8\0"
#define LZW_CLEARCODE "\x80\0"

// hamamatsu
#define NDPI_FORMAT_FLAG    65420
#define NDPI_SOURCELENS     65421

typedef enum file_format {
    aperio_svs,
    hamamatsu_ndpi,
    histech_mirax,
    unknown_format,
    invalid
} file_format;

struct ini_entry {
    char *key;
    char *value;
};

struct ini_group {
    char* group_identifier;
    int32_t start_line;
    int32_t entry_count;
    struct ini_entry *entries;
};

struct ini_file {
    int32_t group_count;
    struct ini_group *groups;
};

struct mirax_level {
    int32_t id;
    int32_t layer_id;
    int32_t record;
    char *key_prefix;
    char *name;
    char *section_key;
    char *section;
};

struct mirax_layer {
    char *layer_name;
    int32_t level_count;
    struct mirax_level **levels;
};

struct mirax_file {
    int32_t all_records_count;
    int32_t count_layers;
    struct mirax_layer **layers;
};

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

#endif