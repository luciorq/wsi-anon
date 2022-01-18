#include "file-api.h"

#include <stdio.h>

struct file_s {
    FILE *fp;
};

file_t *file_open(const char *filename, const char *mode) {
    file_t *stream = NULL;

    FILE *fp = fopen(filename, mode);

    if (fp != NULL) {
        stream = (file_t *)malloc(sizeof(file_t));
        stream->fp = fp;
    }

    return stream;
}

size_t file_read(void *buffer, size_t element_size, size_t element_count, file_t *stream) {
    return fread(buffer, element_size, element_count, stream->fp);
}

char *file_gets(char *buffer, int max_count, file_t *stream) {
    return fgets(buffer, max_count, stream->fp);
}

int file_getc(file_t *stream) { return fgetc(stream->fp); }

int file_seek(file_t *stream, long offset, int origin) { return fseek(stream->fp, offset, origin); }

size_t file_write(const void *buffer, size_t size, size_t count, file_t *stream) {
    return fwrite(buffer, size, count, stream->fp);
}

int file_putc(int character, file_t *stream) { return fputc(character, stream->fp); }

int file_printf(file_t *stream, const char *format, const char *value) {
    return fprintf(stream->fp, format, value);
}

long file_tell(file_t *stream) { return ftell(stream->fp); }

int file_close(file_t *stream) {
    int result = fclose(stream->fp);
    free(stream);
    return result;
}
