#include "tiff-based-io.h"

static const char DOT_SVS[] = ".svs";
static const char SVS[] = "svs";
static const char TIF[] = "tif";
static const char DOT_NDPI[] = ".ndpi";
static const char NDPI[] = "ndpi";

// initialize directory array for a given tiff file
void init_tiff_file(struct tiff_file *file, size_t init_size) {
    size_t alloc_size = init_size * sizeof(struct tiff_directory);
    file->directories = (struct tiff_directory *)malloc(alloc_size);
    memset(file->directories, 0, alloc_size);
    file->used = 0;
    file-> size = init_size;
}

// add directory to a given tiff file and resize array if necessary
void insert_dir_into_tiff_file(struct tiff_file *file, 
        struct tiff_directory *dir) {
    // reallocate directories array dynamically
    if(file->used == file->size) {
        file->size *= 2;
        size_t realloc_size = file->size * sizeof(struct tiff_directory);
        file->directories = (struct tiff_directory *)realloc(file->directories, realloc_size);
        memset(&(file->directories[file->size / 2]), 0, realloc_size / 2);
    }
    file->directories[file->used++] = *dir;
}

// destroy tiff file with associated directories
void free_tiff_file(struct tiff_file *file) {
    free(file->directories);
    file->directories = NULL;
    file->used = file->size = 0;
}

// fix the byte order for data array depending on the endianess
// of the operating system and the tiff file
void fix_byte_order(void *data,
        int32_t size, 
        int64_t count, 
        bool big_endian) {
    // we only need to swap data, if system endianness 
    // and tiff endianness are unequal
    if(is_system_big_endian() != big_endian) {
        switch(size) {
            case 1: break;
            case 2: {
                uint16_t *arr = (uint16_t *)data;
                for (int64_t i = 0; i < count; i++) {
                    _swap_uint16(arr[i]);
                }
                break;
            }
            case 4: {
                uint32_t *arr = (uint32_t *)data;
                for (int64_t i = 0; i < count; i++) {
                    _swap_uint32(arr[i]);
                }
                break;
            }
            case 8: {
                uint64_t *arr = (uint64_t *)data;
                for (int64_t i = 0; i < count; i++) {
                    _swap_uint64(arr[i]);
                }
                break;
            }
            default: break;
        }
    }
}

// read an unsigned integer from a filestream at current position
uint64_t read_uint(file_t *fp, int32_t size, bool big_endian) {
    uint8_t buffer[size];
    if (file_read(buffer, size, 1, fp) != 1) {
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

// get the type size needed to readout the value
uint32_t get_size_of_value(uint16_t type, uint64_t *count) {
    if(type == TIFF_BYTE 
        || type == TIFF_ASCII 
        || type == TIFF_SBYTE 
        || type == TIFF_UNDEFINED) {
        return 1;
    } else if(type == TIFF_SHORT 
        || type == TIFF_SSHORT) {
        return 2;
    } else if(type == TIFF_LONG 
        || type == TIFF_SLONG 
        || type == TIFF_FLOAT 
        || type == TIFF_IFD) {
        return 4;
    } else if(type == TIFF_RATIONAL 
        || type == TIFF_SRATIONAL) {
        *count *= 2;
        return 4;
    } else if(type == TIFF_DOUBLE 
        || type == TIFF_LONG8 
        || type == TIFF_SLONG8 
        || type == TIFF_IFD8) {
        return 8;
    } else {
        return 0;
    }
}

uint64_t fix_ndpi_offset(uint64_t directory_offset, uint64_t offset) {
    // we need to fix the ndpi offset to prevent the pointer from overflowing
    uint64_t new_offset = (directory_offset & ~(uint64_t) UINT64_MAX) 
                            | (offset & UINT32_MAX);
    if(new_offset >= directory_offset) {
        new_offset = min(new_offset - UINT32_MAX - 1, new_offset);
    }
    return new_offset;
}

// read a tiff directory at a certain offset
struct tiff_directory *read_tiff_directory(file_t *fp, 
        int64_t *dir_offset,
        int64_t *in_pointer_offset,
        struct tiff_directory *first_directory, 
        bool big_tiff, 
        bool ndpi, 
        bool big_endian) {
    int64_t offset = *dir_offset;
    *dir_offset = 0;
    
    // seek to directory offset
    if(file_seek(fp, offset, SEEK_SET) != 0) {
        fprintf(stderr, "Error: seeking to offset failed\n");
        return NULL;
    } 

    // read number of entries in directory
    uint64_t entry_count = read_uint(fp, big_tiff ? 8 : 2, big_endian);

    struct tiff_directory *tiff_dir = (struct tiff_directory *)malloc(
        sizeof(struct tiff_directory));
    tiff_dir->count = entry_count;
    tiff_dir->in_pointer_offset = *in_pointer_offset;

    struct tiff_entry *entries = (struct tiff_entry *)malloc(
        entry_count * sizeof(struct tiff_entry));

    for (uint64_t i = 0; i < entry_count; i++) {
        struct tiff_entry *entry = (struct tiff_entry *)malloc(
            sizeof(struct tiff_entry));

        if(entry == NULL) {
            fprintf(stderr, "Error: could not allocate memory for entry\n");
            return NULL;
        }

        entry->start = file_tell(fp);

        uint16_t tag = read_uint(fp, 2, big_endian);
        uint16_t type = read_uint(fp, 2, big_endian);
        uint64_t count = read_uint(fp, big_tiff ? 8 : 4, big_endian);
        entry->tag = tag;
        entry->type = type;
        entry->count = count;

        // calculate the size of the entry value
        uint32_t value_size = get_size_of_value(entry->type, &entry->count);

        if(!value_size) {
            fprintf(stderr, "Error: calculating value size failed\n");
            return NULL;
        }

        // read entry value to array
        uint8_t value[big_tiff ? 8 : 4];
        if (file_read(value, sizeof(value), 1, fp) != 1) {
            fprintf(stderr, "Error: reading value to array failed\n");
            return NULL;
        }

        if(big_tiff) {
            // big tiff offset pointer reserves 8 bytes
            memcpy(&entry->offset, value, 8);
            fix_byte_order(&entry->offset, sizeof(entry->offset), 1, big_endian);
        } else {
            // non big tiff offset only reserves 4 bytes
            int32_t offset32;
            memcpy(&offset32, value, 4);
            fix_byte_order(&offset32, sizeof(offset32), 1, big_endian);
            entry->offset = offset32;
        }
        if(ndpi) {
            struct tiff_entry *first_entry_of_dir = NULL;
            if(first_directory) {
                // retrieve the first entry of the first directory
                for (uint64_t j = 0; j < first_directory->count; j++) {
                    if(first_directory->entries[j].tag == tag) {
                        first_entry_of_dir = &first_directory->entries[j];
                        break;
                    }
                }
            } 
            
            // fix the ndpi offset if we are in the first directory 
            // or the offsets diverge
            if(!first_entry_of_dir 
                || first_entry_of_dir->offset != entry->offset) {
                entry->offset = fix_ndpi_offset(offset, entry->offset);
            }
        }
        entries[i] = *entry;
    }

    // get the directory offset of the successor
    int64_t next_dir_offset = file_tell(fp) + 8;

    tiff_dir->entries = entries;
    tiff_dir->out_pointer_offset = next_dir_offset;
    *dir_offset =  read_uint(fp, (big_tiff || ndpi) ? 8: 4, big_endian);

    return tiff_dir;
}

// check tiff file header
int32_t check_file_header(file_t *fp, bool *big_endian, bool *big_tiff) {
    int32_t result = -1;
    uint16_t endianess;

    // read endianness in header
    if(file_read(&endianess, sizeof endianess, 1, fp) != 1) {
        return result;
    }
            
    if(endianess == TIFF_BIGENDIAN || endianess == TIFF_LITTLEENDIAN) {
        // set endianness and check ndpi version in header
        *big_endian = (endianess == TIFF_BIGENDIAN);
        uint16_t version = read_uint(fp, 2, *big_endian);

        if(version == TIFF_VERSION_BIG) {
            // try to read offset and padding for tiff version
            // if version is not classic
            uint16_t offset_size = read_uint(fp, 2, *big_endian);
            uint16_t pad = read_uint(fp, 2, *big_endian);
            if(offset_size == 8 && pad == 0) {
                *big_tiff = true;
                result = 0;
            }
        }  else if(version == TIFF_VERSION_CLASSIC) {
            *big_tiff = false;
            result = 0;
        }
    }
    return result;
}

// read the tiff file structure with offsets from the file stream
struct tiff_file *read_tiff_file(file_t *fp, bool big_tiff, 
        bool ndpi, bool big_endian) {
    // get directory offset; file stream pointer must be located just 
    // before the directory offset
    int64_t in_pointer_offset = file_tell(fp);
    int64_t diroff = read_uint(fp, 8, big_endian);
    // reading the initial directory
    struct tiff_directory *prev_dir = NULL;
    struct tiff_directory *dir = read_tiff_directory(
        fp, &diroff, &in_pointer_offset, prev_dir, big_tiff, ndpi, big_endian);

    if(dir == NULL) {
        fprintf(stderr, "Error: Failed reading directory.\n");
        return NULL;
    }

    struct tiff_file *file = (struct tiff_file *)malloc(sizeof(struct tiff_file));
    // initialize tiff file and add first directory
    init_tiff_file(file, 1);
    insert_dir_into_tiff_file(file, dir);
    // set the current directory as previous directory
    //struct tiff_directory *prev_dir = dir;

    // when the directory offset is 0 we reached the end of the tiff file
    while(diroff != 0) {
        int64_t current_in_pointer_offset = file_tell(fp) - 8;
        struct tiff_directory *current_dir = read_tiff_directory(fp, &diroff, 
            &current_in_pointer_offset, prev_dir, big_tiff, ndpi, big_endian);

        if(current_dir == NULL) {
            fprintf(stderr, "Error: Failed reading directory.\n");
            return NULL;
        }
        insert_dir_into_tiff_file(file, current_dir);
        prev_dir = current_dir;
    }

    return file;
}


// retrieve the label directory from the tiff file structure
int32_t get_hamamatsu_label_dir(struct tiff_file *file, 
        file_t *fp, 
        bool big_endian) {
    for (uint64_t i = 0; i < file->used; i++) {
        struct tiff_directory temp_dir = file->directories[i];
            
        for(uint64_t j = 0; j < temp_dir.count; j++) {
            struct tiff_entry temp_entry = temp_dir.entries[j];

            if(temp_entry.tag == NDPI_SOURCELENS) {
                int32_t entry_size = get_size_of_value(
                    temp_entry.type, &temp_entry.count);

                if(entry_size) {

                    if(temp_entry.type == FLOAT) {
                        float *v_buffer = (float *)malloc(
                            entry_size * temp_entry.count);

                        // we need to step 8 bytes from start pointer 
                        // to get the expected value
                        uint64_t new_start = temp_entry.start + 8;
                        if(file_seek(fp, new_start, SEEK_SET)) {
                            fprintf(stderr, "Error: Failed to seek to offset %lu.\n", new_start);
                            return -1;
                        }
                        if(file_read(v_buffer, entry_size, temp_entry.count, fp) != 1) {
                            fprintf(stderr, "Error: Failed to read entry value.\n");
                            return -1;
                        }
                        fix_byte_order(v_buffer, sizeof(float), 1, big_endian);
                        
                        if(*v_buffer == -1) {
                            // we found the label directory
                            return i;                       
                        }
                    }
                }
            }
        }
    }
    return -1;
}

// read a pointer from the directory entries by tiff tag
uint32_t *read_pointer_by_tag(file_t *fp, 
        struct tiff_directory *dir, 
        int tag, 
        bool ndpi, 
        bool big_endian, 
        int32_t *length) {
    for(uint64_t i  = 0; i < dir->count; i++) {
        struct tiff_entry entry = dir->entries[i];
        if(entry.tag == tag) {
            int32_t entry_size = get_size_of_value(entry.type, &entry.count);

            if(entry_size) {
                uint32_t *v_buffer = (uint32_t *)malloc(entry_size * entry.count);

                if(entry.count == 1) {
                    *length = entry.count;
                    v_buffer[0] = entry.offset;
                    return v_buffer;
                }

                uint64_t new_offset = entry.offset;

                if(ndpi) {
                    new_offset = entry.start + 8;
                }

                if(file_seek(fp, new_offset, SEEK_SET)) {
                    fprintf(stderr, "Error: Failed to seek to offset %lu.\n", entry.offset);
                    continue;
                }
                if(file_read(v_buffer, entry_size, entry.count, fp) < 1) {
                    fprintf(stderr, "Error: Failed to read entry value.\n");
                    continue;
                }

                fix_byte_order(v_buffer, entry_size, entry.count, big_endian);
                *length = entry.count;

                return v_buffer;
            }
        }
    }

    return NULL;
}

int32_t wipe_label(file_t *fp, 
        struct tiff_directory *dir, 
        bool ndpi, 
        bool big_endian, 
        const char *prefix) {
    int32_t size_offsets;
    int32_t size_lengths;
    // gather strip offsets and lengths form tiff directory
    uint32_t *strip_offsets = read_pointer_by_tag(
        fp, dir, TIFFTAG_STRIPOFFSETS, ndpi, big_endian, &size_offsets);
    uint32_t *strip_lengths = read_pointer_by_tag(
        fp, dir, TIFFTAG_STRIPBYTECOUNTS, ndpi, big_endian, &size_lengths);

    if(strip_offsets == NULL || strip_lengths == NULL) {
        fprintf(stderr, "Error: Could not retrieve strip offset and length.\n");
        return -1;
    }

    if(size_offsets != size_lengths) {
        fprintf(stderr, "Error: Length of strip offsets and lengths are not matching.\n");
        return -1;
    }

    for(int32_t i = 0; i < size_offsets; i++) {
        file_seek(fp, strip_offsets[i], SEEK_SET);

        if(prefix != NULL) {
            // we check the head of the directory offset for a given 
            // prefix. if the head is not equal to the given prefix 
            // we do not wipe the label data
            size_t prefix_len = strlen(prefix);
            char *buf =  (char *)malloc(prefix_len + 1);
            buf[prefix_len] = '\0';

            if(file_read(buf, prefix_len, 1, fp) != 1) {
                fprintf(stderr, "Error: Could not read strip prefix.\n");
                free(buf);
                return -1;
            }

            if(strcmp(prefix, buf) != 0) {
                fprintf(stderr, "Error: Prefix in data strip not found.\n");
                return -1;
            }
            
            file_seek(fp, strip_offsets[i], SEEK_SET);
            free(buf);
        }
        
        // fill strip with zeros
        char *strip = get_empty_char_buffer("0", strip_lengths[i], prefix);
        if(!file_write(strip, 1, strip_lengths[i], fp)) {
            fprintf(stderr, "Error: Wiping image data failed.\n");
            free(strip);
            return -1;
        }
        free(strip);
    }
    
    return 0;
}

int32_t unlink_label_directory(file_t *fp, 
        struct tiff_file *file, 
        int32_t current_dir,
        bool is_ndpi) {
    struct tiff_directory dir = file->directories[current_dir];
    struct tiff_directory successor = file->directories[current_dir+1];

    if(!is_ndpi && successor.count == 0 && successor.in_pointer_offset == 0) {
        // current directory is the last in file
        // search search to out pointer of current dir
        if(file_seek(fp, dir.out_pointer_offset, SEEK_SET)) {
            fprintf(stderr, "Error: Failed to seek to offset.\n");
            return -1;
        }
        // overwrite out pointer with 0 to end file
        uint64_t new_pointer_address[1];
        new_pointer_address[0] = 0x0;
        if(file_write(new_pointer_address, sizeof(uint64_t), 1, fp) != 1) {
            fprintf(stderr, "Error: Failed to write directory out pointer \
                        to null at pointer position.\n");
            return -1;
        }
    } else {
        // current directory has a successor
        if(file_seek(fp, successor.in_pointer_offset, SEEK_SET)) {
            fprintf(stderr, "Error: Failed to seek to offset.\n");
            return -1;
        }
        uint64_t new_pointer_address[1];
        if(file_read(&new_pointer_address, sizeof(uint64_t), 1, fp) != 1) {
            fprintf(stderr, "Error: Failed to read pointer.\n");
            return -1;
        }
        if(file_seek(fp, dir.in_pointer_offset, SEEK_SET)) {
            fprintf(stderr, "Error: Failed to seek to offset.\n");
            return -1;
        }
        if(file_write(new_pointer_address, sizeof(uint64_t), 1, fp) != 1) {
            fprintf(stderr, "Error: Failed to write directory in pointer \
                        to predecessor at pointer position.\n");
            return -1;
        }
    } 
    
    return 0;
}

const char *duplicate_file(const char *filename, 
        const char *new_label_name,
        const char *file_extension) {
    // retrive filename from whole file path
    const char* _filename = get_filename_from_path(filename);

    if(_filename == NULL) {
        fprintf(stderr, "Error: Could not retrieve filename from filepath.\n");
        return NULL;
    }

    // get the directory path
    int32_t l_filename = strlen(filename);
    int32_t diff = l_filename - strlen(_filename);
    char path[diff + 1];
    memcpy(path, &filename[0], diff);
    path[diff] = '\0';

    const char *new_filename;
    // now we can concat the new filename
    if(new_label_name == NULL) {
        // if no label is given, we give the file a generic name
        const char *dummy_filename = "anonymized-wsi";
        new_filename = concat_path_filename_ext(path, dummy_filename, file_extension);
    } else {
        new_filename = concat_path_filename_ext(path, new_label_name, file_extension);
    }

    // we copy the file in our current directory
    // create a subfolder /output/?
    if(new_filename != NULL && copy_file_v2(filename, new_filename) == 0) {
        return new_filename;
    } else {
        return NULL;
    }
}

int32_t handle_hamamatsu(const char **filename, 
        const char *new_label_name, 
        bool disable_unlinking,
        bool do_inplace) {
    fprintf(stdout, "Anonymize Hamamatsu WSI...\n");
    if(!do_inplace) {
        *filename = duplicate_file(*filename, new_label_name, DOT_NDPI);
    }

    file_t *fp;
    fp = file_open(*filename, "r+");

    bool big_tiff = false;
    bool big_endian = false;
    // we check the header again, so the pointer 
    // will be placed at the expected position
    int result = check_file_header(fp, &big_endian, &big_tiff);

    if (result != 0) {
        fprintf(stderr, "Error: Could not read header file.\n");
        file_close(fp);
        return result;
    }

    // read the file structure
    struct tiff_file *file;
    file = read_tiff_file(fp, big_tiff, true, big_endian);

    // find the label directory
    int dir_count = get_hamamatsu_label_dir(file, fp, big_endian);
    if(dir_count == -1) {
        fprintf(stderr, "Error: No label directory.\n");
        free_tiff_file(file);
        file_close(fp);
        return -1;
    }

    struct tiff_directory dir = file->directories[dir_count];

    // wipe label data from directory
    // check for JPEG SOI header in label dir
    result = wipe_label(fp, &dir, true, big_endian, JPEG_SOI);

    if (result != 0) {
        free_tiff_file(file);
        file_close(fp);
        return result;
    }

    if(!disable_unlinking) {
        // unlink the empty label directory from file structure
        result = unlink_label_directory(fp, file, dir_count, true);
    }
    
    free_tiff_file(file);
    file_close(fp);

    return result;
}

// get aperio directory for aperio AT2 and older
int32_t get_aperio_dir_by_name(file_t *fp,
        struct tiff_file *file,
        const char *dir_name) {
    for(uint64_t i = 0; i < file->used; i++) {
        
        struct tiff_directory dir = file->directories[i];
        for(uint64_t j = 0; j < dir.count; j++) {
            struct tiff_entry entry = dir.entries[j];
            if(entry.tag == TIFFTAG_IMAGEDESCRIPTION) {            
                // get the image description from file
                file_seek(fp, entry.offset, SEEK_SET);
                int32_t entry_size = get_size_of_value(
                    entry.type, &entry.count);

                char buffer[entry_size * entry.count];
                if(file_read(&buffer, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not read image description.\n");
                    return -1;
                }

                // search for Aperio and given tag in description
                if(contains(buffer, "Aperio") && contains(buffer, dir_name)) {
                    return i;
                }
            }
        }
    }
    return -1;
}

bool is_aperio_gt450(file_t *fp, struct tiff_file *file) {
    for(uint64_t i = 0; i < file->used; i++) {
        struct tiff_directory dir = file->directories[i];
        for(uint64_t j = 0; j < dir.count; j++) {
            struct tiff_entry entry = dir.entries[j];
            if(entry.tag == TIFFTAG_IMAGEDESCRIPTION) {            
                // get the image description from file
                file_seek(fp, entry.offset, SEEK_SET);
                int32_t entry_size = get_size_of_value(
                    entry.type, &entry.count);

                char buffer[entry_size * entry.count];
                if(file_read(&buffer, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not read image description.\n");
                    return false;
                }

                // search for Aperio GT450
                if(contains(buffer, "Aperio Leica Biosystems GT450")) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool dir_contains_image_description(struct tiff_directory *dir) {
    for(uint64_t i = 0; i < dir->count; i++) {
        struct tiff_entry entry = dir->entries[i];
        if(entry.tag == TIFFTAG_IMAGEDESCRIPTION) {
            return true;
        }
    }
    return false;
}


int32_t get_aperio_gt450_dir_by_name(file_t *fp, 
        struct tiff_file *file,
        const char *dir_name) {
    for(uint64_t i = 0; i < file->used; i++) {
        
        struct tiff_directory dir = file->directories[i];
        //printf("--directory %i\n", i);
        for(uint64_t j = 0; j < dir.count; j++) {

            struct tiff_entry entry = dir.entries[j];
            
            if(entry.tag == TIFFTAG_SUBFILETYPE) {
                if(entry.offset == 0) { // thumbnail or else
                    break; // skip IFD
                }

                if ((strcmp(dir_name, "label") == 0 && entry.offset == 1)||
                    (strcmp(dir_name, "macro") == 0 && entry.offset == 9)) {
                    return i;
                }
            }
        }
    }
    return -1;
}

int32_t change_macro_image_compression_gt450(file_t *fp, 
        struct tiff_file *file,
        int32_t directory) {
    // macro image for gt450 needs to be treated differently because it is JPEG encoded
    // therefore we need to convert it to LZW compression
    struct tiff_directory dir = file->directories[directory];
    for(int i = 0; i < dir.count; i++) {
        struct tiff_entry entry = dir.entries[i];
        if(entry.tag == TIFFTAG_COMPRESSION) {
            if(file_seek(fp, entry.start + 12, SEEK_SET)) {
                fprintf(stderr, "Error: Failed to seek to offset %lu.\n", entry.offset);
                continue;
            }
            uint64_t lzw_com = COMPRESSION_LZW;
            if(!file_write(&lzw_com, 1, sizeof(uint64_t), fp)) {
                fprintf(stderr, "Error: Wiping image data failed.\n");
                return -1;
            }
            break;
        }
    }
    return 0;
}

int32_t wipe_and_unlink_directory(file_t *fp, 
        struct tiff_file *file,
        int32_t directory,
        bool big_endian,
        bool is_aperio_gt450,
        bool disable_unlinking,
        const char *prefix) {
    if(directory == -1) {
        return -1;
    }

    struct tiff_directory dir = file->directories[directory];

    int32_t result = 0;
    result = wipe_label(fp, &dir, false, big_endian, prefix);

    if(result == -1) {
        return -1;
    }

    // unlinking works also for gt450?
    if(!disable_unlinking) {
        fprintf(stdout, "Unlink DIR \n");
        // TODO: unlinking of label leads to displaying thumbnail as label
        // TODO: unlinking of macro image not working
        result = unlink_label_directory(fp, file, directory, false);
    }

    return result;
}

int32_t handle_aperio(const char **filename, 
        const char *new_label_name, 
        bool keep_macro_image,
        bool disable_unlinking,
        bool do_inplace) {
    fprintf(stdout, "Anonymize Aperio WSI...\n");

    if(!do_inplace) {
        // check if filename is svs or tif here
        *filename = duplicate_file(*filename, new_label_name, DOT_SVS);      
    }

    file_t *fp;
    fp = file_open(*filename, "r+");

    bool big_tiff = false;
    bool big_endian = false;
    int32_t result = check_file_header(fp, &big_endian, &big_tiff);

    if(result != 0) {
        file_close(fp);
        return result;
    }

    struct tiff_file *file;
    file = read_tiff_file(fp, big_tiff, false, big_endian);

    if(file == NULL) {
        fprintf(stderr, "Error: Could not read tiff file.\n");
        return -1;
    }

    bool _is_aperio_gt450 = is_aperio_gt450(fp, file);

    // delete label image
    int32_t label_dir = 0;
    if(_is_aperio_gt450) {
        label_dir = get_aperio_gt450_dir_by_name(fp, file, "label");
    } else {
        label_dir = get_aperio_dir_by_name(fp, file, "label");
    }

    if(label_dir == -1) {
        fprintf(stderr, "Error: Could not find IFD of label image.\n");
        return -1;
    }
    
    result = wipe_and_unlink_directory(fp, file, label_dir,
            big_endian, _is_aperio_gt450, disable_unlinking, LZW_CLEARCODE);

    if(result != 0) {
        free_tiff_file(file);
        file_close(fp);
        return result;
    }

    // delete macro image
    if(!keep_macro_image) {
        int32_t macro_dir = -1;
        if(_is_aperio_gt450) {
            macro_dir = get_aperio_gt450_dir_by_name(fp, file, "macro");
        } else {
            macro_dir = get_aperio_dir_by_name(fp, file, "macro");
        }

        if(macro_dir == -1) {
            fprintf(stderr, "Error: Could not find IFD of macro image.\n");
            return -1;
        }

        result = wipe_and_unlink_directory(fp, file, macro_dir, 
            big_endian, _is_aperio_gt450, disable_unlinking, NULL);

        if(_is_aperio_gt450) {
            result = change_macro_image_compression_gt450(fp, file, macro_dir);
        }

        if(result != 0) {
            free_tiff_file(file);
            file_close(fp);
            return result;
        }
    }

    // clean up
    free_tiff_file(file);
    file_close(fp);
    return result;
}

int32_t is_hamamatsu(const char *filename) {
    int result = 0;
    const char *ext = get_filename_ext(filename);

    // check for valid file extension
    if(strcmp(ext, NDPI) != 0) {
        return result;
    }

    // check if ndpi tiff tags are present
    file_t *fp = file_open(filename, "r");
    bool big_tiff = false;
    bool big_endian = false;
    result = check_file_header(fp, &big_endian, &big_tiff);
    file_close(fp);

    return (result == 0);
}

int32_t has_aperio_tag(file_t *fp, struct tiff_file *file) {
    for(uint64_t i = 0; i < file->used; i++) {
        struct tiff_directory dir = file->directories[i];
        for(uint64_t j = 0; j < dir.count; j++) {
            struct tiff_entry entry = dir.entries[j];
            if(entry.tag == TIFFTAG_IMAGEDESCRIPTION) {            
                // get the image description from file
                file_seek(fp, entry.offset, SEEK_SET);
                int32_t entry_size = get_size_of_value(
                    entry.type, &entry.count);

                char buffer[entry_size * entry.count];
                if(file_read(&buffer, entry.count, entry_size, fp) != 1) {
                    fprintf(stderr, "Error: Could not read image description.\n");
                    return -1;
                }

                // search for Aperio and given tag in description
                if(contains(buffer, "Aperio")) {
                    return 1;
                }
            }
        }
    }
    return -1;
}

int32_t is_aperio(const char *filename) {
    int32_t result = 0;
    const char *ext = get_filename_ext(filename);

    // check for valid file extension
    if(strcmp(ext, SVS) != 0 && strcmp(ext, TIF) != 0) {
        return result;
    }

    file_t *fp;
    fp = file_open(filename, "r+w");

    if(fp == NULL) {
        fprintf(stderr, "Error: Could not open tiff file.\n");
        return result;
    }

    bool big_tiff = false;
    bool big_endian = false;
    result = check_file_header(fp, &big_endian, &big_tiff);

    if(result != 0) {
        return result;
    }

    struct tiff_file *file;
    file = read_tiff_file(fp, big_tiff, false, big_endian);

    if(file == NULL) {
        fprintf(stderr, "Error: Could not read tiff file.\n");
        file_close(fp);
        return result;
    }

    result = has_aperio_tag(fp, file);

    if (result == -1) {
        fprintf(stderr, "Error: Could not find aperio label directory.\n");
    }
    
    // is aperio
    file_close(fp);
    return (result == 1);
}
