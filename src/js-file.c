#include "file-api.h"

#include <emscripten.h>
#include <stdio.h>
#include <string.h>

struct file_s {
    char *filename;
    char *mode;
    long offset;
    size_t size;
};

EM_ASYNC_JS(size_t, get_chunk, (void *buffer, size_t size, const char *filename, long offset), {
    const jsFilename = UTF8ToString(filename);
    const anonStream = AnonymizedStream.retrieve(jsFilename);
    const sliceData = await anonStream.getAnonymizedChunk(offset, size);
    const sourceView = new Uint8Array(sliceData, 0, sliceData.byteLength);
    const targetView = new Uint8Array(wasmMemory.buffer, buffer, sliceData.byteLength);
    targetView.set(sourceView);
    return sliceData.byteLength;
});

EM_ASYNC_JS(void, set_chunk, (const void *buffer, size_t size, const char *filename, long offset), {
    const sourceView = new Uint8Array(wasmMemory.buffer, buffer, size);
    const targetBuffer = new ArrayBuffer(size);
    const targetView = new Uint8Array(targetBuffer);
    targetView.set(sourceView);
    const jsFilename = UTF8ToString(filename);
    const anonStream = AnonymizedStream.retrieve(jsFilename);
    anonStream.addChanges(targetBuffer, offset)
});

EM_ASYNC_JS(int32_t, file_present_in_form, (const char *filename), {
    const jsFilename = UTF8ToString(filename);
    return AnonymizedStream.exists(jsFilename) ? 1 : 0;
});

EM_ASYNC_JS(size_t, file_size, (const char *filename), {
    const jsFilename = UTF8ToString(filename);
    const anonStream = AnonymizedStream.retrieve(jsFilename);
    return anonStream.size;
});

file_t *file_open(const char *filename, const char *mode) {
    file_t *stream = NULL;

    if (file_present_in_form(filename)) {
        stream = (file_t *)malloc(sizeof(file_t));
        stream->offset = 0;
        stream->size = file_size(filename);
        stream->filename = (char *)malloc(strlen(filename) + 1);
        stream->mode = (char *)malloc(strlen(mode) + 1);
        strcpy(stream->filename, filename);
        strcpy(stream->mode, mode);
    }

    return stream;
}

size_t file_read(void *buffer, size_t element_size, size_t element_count, file_t *stream) {
    get_chunk(buffer, element_size * element_count, stream->filename, stream->offset);
    // TODO: how many elements were really read and adapt offset accordingly
    stream->offset += element_size * element_count;
    return element_count;
}

char *file_gets(char *buffer, int32_t max_count, file_t *stream) {
    if (stream->offset == stream->size) {
        return NULL;
    }
    if (max_count == 1) {
        buffer[0] = '\0';
        return buffer;
    }

    size_t chunk_size = get_chunk(buffer, max_count - 1, stream->filename, stream->offset);

    char *nl = strchr(buffer, '\n');
    if (nl == NULL) {
        buffer[chunk_size] = '\0';
        stream->offset += chunk_size;
    } else {
        *(nl + 1) = '\0';
        stream->offset = stream->offset + (nl - buffer) + 1;
    }
    return buffer;
}

int32_t file_getc(file_t *stream) {
    char c;
    if (stream->offset == stream->size) {
        return EOF;
    }
    get_chunk(&c, 1, stream->filename, stream->offset);
    stream->offset += 1;
    return c;
}

int32_t file_seek(file_t *stream, long offset, int32_t origin) {
    long new_offset = 0;
    if (origin == SEEK_SET) {
        new_offset = offset;
    } else if (origin == SEEK_CUR) {
        new_offset = stream->offset + offset;
    } else {
        new_offset = stream->size + offset;
    }
    if ((new_offset < 0) || (new_offset > stream->size)) {
        return -1;
    }

    stream->offset = new_offset;
    return 0;
}

size_t file_write(const void *buffer, size_t size, size_t count, file_t *stream) {
    // TODO: if file not writable, return 0 immediately
    set_chunk(buffer, size * count, stream->filename, stream->offset);
    // TODO: how many bytes were really written -> adapt offset accordingly
    stream->offset += (size * count);
    // TODO: check under which conditions offset is updated for writing (could
    // be that there were differences according to open mode)
    return count;
}

int32_t file_putc(int32_t character, file_t *stream) {
    // TODO: if file not writable, return 0 immediately
    char c = (char)character; // to be sure to avoid endianess problems
    set_chunk(&c, 1, stream->filename, stream->offset);
    // TODO: check offset update conditions (see comment in file_write)
    stream->offset += 1;
    // TODO: handle failures
    return c;
}

int32_t file_printf(file_t *stream, const char *format, const char *value) {
    int32_t buffer_size = snprintf(NULL, 0, format, value); // dry run to find out buffer size
    char *buffer = (char *)malloc(buffer_size + 1);
    sprintf(buffer, format, value);
    set_chunk(buffer, buffer_size, stream->filename, stream->offset);
    free(buffer);
    stream->offset += buffer_size;
    // TODO: handle edge cases (partial writes, errors, etc.)
    return buffer_size;
}

uint64_t file_tell(file_t *stream) { return stream->offset; }

int32_t file_close(file_t *stream) {
    free(stream->mode);
    free(stream->filename);
    free(stream);
    return 0;
}
