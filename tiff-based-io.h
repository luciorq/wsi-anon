#ifndef HEADER_TIFF_IO_H
#define HEADER_TIFF_IO_H

#include <stdio.h>
#include <stdint.h>

#define TIFF_BIGENDIAN      0x4d4d
#define TIFF_LITTLEENDIAN   0x4949

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

int handle_hamamatsu(const char *filename, const char *new_label_name);
int handle_aperio(const char *filename, const char *new_label_name);
int handle_mirax(const char *filename, const char *new_label_name);

int is_hamamatsu(const char *filename);
int is_aperio(const char *filename);
int is_mirax(const char *filename) ;

int file_exists(const char *filename);

#endif