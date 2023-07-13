#ifndef HEADER_DEFINES_H
#define HEADER_DEFINES_H

#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define UNUSED(x) (void)(x)

// mirax
#define MAX_CHAR_IN_LINE 100
#define MRXS_ROOT_OFFSET_NONHIER 41
#define MRXS_SLIDE_DAT_NONHIER_GROUP_OFFSET 4
#define MRXS_MAX_SIZE_DATA_DAT 1000
#define MRXS_PROFILENAME "ProfileName=\""

// tiff endianness
#define TIFF_BIGENDIAN 0x4d4d
#define TIFF_LITTLEENDIAN 0x4949

// tiff tags
#define TIFF_VERSION_CLASSIC 42
#define TIFF_VERSION_BIG 43
#define TIFFTAG_IMAGEDESCRIPTION 270
#define TIFFTAG_STRIPOFFSETS 273
#define TIFFTAG_STRIPBYTECOUNTS 279
#define TIFFTAG_SUBFILETYPE 254
#define TIFFTAG_COMPRESSION 259
#define TIFFTAG_SOFTWARE 305
#define TIFFTAG_DATETIME 306
#define TIFFTAG_TILEOFFSETS 324
#define TIFFTAG_TILEBYTECOUNTS 325
#define TIFFTAG_XMP 700

// compression type
#define COMPRESSION_LZW 5

// aperio
#define APERIO_FILENAME_TAG "Filename = "
#define APERIO_USER_TAG "User = "
#define APERIO_DATE_TAG "Date = "
#define APERIO_BARCODE_TAG "Barcode = "

// ventana
#define VENTANA_FILENAME_ATT "JP2FileName="
#define VENTANA_UNITNUMBER_ATT "UnitNumber="
#define VENTANA_USERNAME_ATT "UserName="
#define VENTANA_BARCODE1D_ATT "Barcode1D="
#define VENTANA_BARCODE2D_ATT "Barcode2D="
#define VENTANA_BASENAME_ATT "BaseName="
#define VENTANA_BUILDDATE_ATT "BuildDate="

// philips based (Philips' iSyntax and TIFF)
#define PHILIPS_DELIMITER_STR "\"IString\""
#define PHILIPS_DELIMITER_INT "\"IUInt16\""
#define PHILIPS_ATT_END "</Attribute"
#define PHILIPS_ATT_OPEN "<Attribute"
#define PHILIPS_CLOSING_SYMBOL ">"
#define PHILIPS_ATT_PMSVR "PMSVR="
#define PHILIPS_DATETIME_ATT "DICOM_ACQUISITION_DATETIME"
#define PHILIPS_SERIAL_ATT "DICOM_DEVICE_SERIAL_NUMBER"
#define PHILIPS_SLOT_ATT "<Attribute Name=\"PIIM_DP_SCANNER_SLOT_NUMBER"
#define PHILIPS_RACK_ATT "<Attribute Name=\"PIIM_DP_SCANNER_RACK_NUMBER"
#define PHILIPS_OPERID_ATT "PIIM_DP_SCANNER_OPERATOR_ID"
#define PHILIPS_BARCODE_ATT "PIM_DP_UFS_BARCODE"
#define PHILIPS_SOURCE_FILE_ATT "PIM_DP_SOURCE_FILE"
#define PHILIPS_MIN_DATETIME "19000101000000.000000"
#define PHILIPS_LABELIMAGE "LABELIMAGE"
#define PHILIPS_MACROIMAGE "MACROIMAGE"
#define PHILIPS_OBJECT "Object>"
#define PHILIPS_IMAGE_DATA "PIM_DP_IMAGE_DATA"

// iSyntax
#define ISYNTAX_ROOTNODE "DPUfsImport"
#define ISYNTAX_EOT "\r\n\004"
#define ISYNTAX_DATA "</Data"

// hamamatsu
#define NDPI_FORMAT_FLAG 65420
#define NDPI_SOURCELENS 65421
#define NDPI_BIT_EXTENSION 4
#define NDPI_ENTRY_EXTENSION 12

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
} TIFF_DATATYPE;

#define JPEG_SOI "\xff\xd8\xff\xe0\0"
#define JPEG_EOI "\xff\xd9\0"
#define LZW_CLEARCODE "\x80\0"

#define MACRO "macro"
#define LABEL "label"

struct ini_entry {
    const char *key;
    const char *value;
};

struct ini_group {
    const char *group_identifier;
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
    const char *key_prefix;
    const char *name;
    const char *section_key;
    const char *section;
};

struct mirax_layer {
    const char *layer_name;
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
    uint32_t count;
    uint64_t offset;
    uint64_t start;
};

struct tiff_directory {
    struct tiff_entry *entries;
    uint32_t count;
    uint64_t in_pointer_offset;
    uint64_t out_pointer_offset;
    uint64_t number;
    uint32_t ndpi_high_bits;
};

struct tiff_file {
    uint32_t used;
    uint32_t size;
    struct tiff_directory *directories;
};

struct metadata {
    const char *key;
    const char *value;
};

// TODO: members are incomplete --> view concept again or comment in members below!!
struct wsi_data {
    int8_t format;
    const char *filename;
    // struct associated_image_data **label;
    // struct associated_image_data **macro;
    struct metadata *metadata;
};

// TODO: find out if this even makes any sense? all data (except for macro) overwritten?
// TODO: is overwrite_metadata not redundant if flag is set?
struct anon_configuration {
    bool overwrite_label;
    bool overwrite_macro;
    bool overwrite_metadata;
    bool keep_macro_image;
    bool disable_unlinking;
    bool do_inplace;
};

#endif