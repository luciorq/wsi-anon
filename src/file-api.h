#ifndef HEADER_FILE_API_H
#define HEADER_FILE_API_H

#include <stdlib.h>

typedef struct file_s file_t;

file_t *file_open(const char *filename, const char *mode);

size_t file_read(void *buffer, size_t element_size, size_t element_count, file_t *stream);

char *file_gets(char *buffer, int max_count, file_t *stream);

int file_getc(file_t *stream);

int file_seek(file_t *stream, long offset, int origin);

size_t file_write(const void *buffer, size_t size, size_t count, file_t *stream);

int file_putc(int character, file_t *stream);

int file_printf(file_t *stream, const char *format, const char *value);

long file_tell(file_t *stream);

int file_close(file_t *stream);

#endif
