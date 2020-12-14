#include "mirax-io.h"

static const char MRXS_EXT[] = "mrxs";
static const char DOT_MRXS_EXT[] = ".mrxs";
static const char SLIDEDAT[] = "Slidedat.ini";
static const char GENERAL[] = "GENERAL";
static const char SLIDE_NAME[] = "SLIDE_NAME";
static const char PROJECT_NAME[] = "PROJECT_NAME";
static const char HIERARCHICAL[] = "HIERARCHICAL";
static const char INDEXFILE[] = "INDEXFILE";
static const char DATAFILE[] = "DATAFILE";
static const char FILE_COUNT[] = "FILE_COUNT";
static const char NONHIER_COUNT[] = "NONHIER_COUNT";
static const char NONHIER_X_COUNT[] = "NONHIER_%d_COUNT";
static const char NONHIER_X_NAME[] = "NONHIER_%d_NAME";
static const char NONHIER_X_VAL_X[] = "NONHIER_%d_VAL_%d";
static const char NONHIER_X_VAL_X_SECTION[] = 
    "NONHIER_%d_VAL_%d_SECTION";
static const char NONHIER_X_VAL_X_IMAGENUMBER_X[] = 
    "NONHIER_%d_VAL_%d_IMAGENUMBER_X";
static const char NONHIER_X_VAL_X_IMAGENUMBER_Y[] = 
    "NONHIER_%d_VAL_%d_IMAGENUMBER_Y";
static const char _SECTION[] = "_SECTION";
static const char FILE_[] = "FILE_%d";
static const char PROJECT_X[] = "Project (%s)";

// define layer and level name in mirax file
static const char *SCAN_DATA_LAYER = "Scan data layer";
static const char *SLIDE_BARCODE = "ScanDataLayer_SlideBarcode";

char *concat_wildcard_string_int32(const char *str, 
        int32_t integer) {
    char *result_string = (char *)malloc(
        strlen(str) + number_of_digits(integer) + 1);
    sprintf(result_string, str, integer);
    return result_string;
}

char *concat_wildcard_string_m_int32(const char *str, 
        int32_t integer1, 
        int32_t integer2) {
    char *result_string = (char *)malloc(strlen(str) 
                            + number_of_digits(integer1) 
                            + number_of_digits(integer2) + 1);
    sprintf(result_string, str, integer1, integer2);
    return result_string;
}

char *concat_wildcard_string_string(const char *wildcard_str,
        const char *replacement) {
    char *result_string = (char *)malloc(strlen(wildcard_str)
                            + strlen(replacement) + 1);
    sprintf(result_string, wildcard_str, replacement);
    return result_string;
}

// retrieve the file structure of the mirax file from the
// Slidedat.ini file
struct mirax_file *get_mirax_file_structure(struct ini_file *ini, 
        int32_t l_count) {
    // initialize mirrax file and array of associated layers
    struct mirax_file *mirax_file = (struct mirax_file *)malloc(
        sizeof(struct mirax_file));

    struct mirax_layer **layers = (struct mirax_layer **)malloc(
        l_count * sizeof(struct mirax_layer));

    int32_t records = 0;
    for(int32_t layer_id = 0; layer_id < l_count; layer_id++) {
        // initialize and populate new layer
        struct mirax_layer *layer = (struct mirax_layer *)malloc(
            sizeof(struct mirax_layer));

        char *nonhier_level_count_key = concat_wildcard_string_int32(
            NONHIER_X_COUNT, 
            layer_id);

        char *nonhier_level_count = get_value_from_ini_file(
            ini, 
            HIERARCHICAL, 
            nonhier_level_count_key);

        int32_t nhl_count;
        sscanf(nonhier_level_count, "%d", &nhl_count);
        layer->level_count = nhl_count;

        char *layer_name_key = concat_wildcard_string_int32(
            NONHIER_X_NAME, 
            layer_id);

        layer->layer_name = get_value_from_ini_file(
            ini, 
            HIERARCHICAL, 
            layer_name_key);

        // initialize and allocate new array of sub lavels of array
        struct mirax_level **levels = (struct mirax_level **)malloc(
            nhl_count * sizeof(struct mirax_level));

        for(int32_t level_id = 0; level_id < nhl_count; level_id++) {
            // initialize and populate sublevels
            //printf("layer %i, level %i\n", layer_id, level_id);
            struct mirax_level *level = (struct mirax_level *)malloc(
                sizeof(struct mirax_level));

            level->id = level_id;
            level->layer_id = layer_id;
            level->record = layer_id + level_id;
            level->key_prefix = concat_wildcard_string_m_int32(
                NONHIER_X_VAL_X, 
                layer_id, 
                level_id);
            level->name = get_value_from_ini_file(
                ini, 
                HIERARCHICAL, 
                level->key_prefix);
            char *section_key = (char *)malloc(
                sizeof(level->key_prefix) + sizeof(_SECTION) + 1);
            strcpy(section_key, level->key_prefix);
            strcat(section_key, _SECTION);
            level->section_key = section_key;
            level->section = get_value_from_ini_file(
                ini, 
                HIERARCHICAL, 
                level->section_key);
            levels[level_id] = level;
            records++;
        }
        // add level array to layer
        layer->levels = levels;
        layers[layer_id] = layer;
    }
    // add layer arrays to mirax file
    mirax_file->all_records_count = records;
    mirax_file->count_layers = l_count;
    mirax_file->layers = layers;
    return mirax_file;
}

// retrive mirax level from file structure by
// layer and level name
struct mirax_level *get_level_by_name(struct mirax_layer **layers, 
        const char *layer_name, 
        const char *level_name) {
    int32_t length = sizeof(**layers) / sizeof(layers[0]);
    for(int32_t i = 0; i < length; i++) {
        struct mirax_layer *layer = layers[i];
        //printf("layer name %s\n", layer->layer_name);
        if(strcmp(layer->layer_name, layer_name) == 0) {
            for(int j = 0; j < layer->level_count; j++) {
                struct mirax_level *level = layer->levels[j];
                //printf("level name %s\n", level->name);
                if(strcmp(level->name, level_name) == 0) {
                    return level;
                }
            }
        }
    }
    return NULL;
}

// read a signed integer 32 from file stream
int32_t *read_int32(FILE *fp) {
    int32_t *buffer = (int32_t *)malloc(sizeof(int32_t));

    if(fread(buffer, sizeof(*buffer), 1, fp) != 1) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

// assert a int32_t has a certain value
// used to skip pointer position on stream
bool assert_value(FILE *fp, int32_t value) {
    int32_t *v_to_check = read_int32(fp);
    return *v_to_check == value;
}

// read file number, position and size frrom index dat
int32_t *read_data_location(char *filename, 
        int32_t record, 
        int32_t **position, 
        int32_t **size) {
    FILE *fp = fopen(filename, "r");

    if(fp == NULL) {
        fprintf(stderr, "Error: Could not open file stream.\n");
        return NULL;
    }
    
    if(fseek(fp, MRXS_ROOT_OFFSET_NONHIER, SEEK_SET)) {
        fprintf(stderr, "Error: Can not seek to mrxs offset in Index.dat.\n");
        fclose(fp);
        return NULL;
    }

    int32_t *table_base = read_int32(fp);
    fseek(fp, (*table_base + (record*4)), SEEK_SET);

    int32_t *list_header = read_int32(fp);
    fseek(fp, *list_header, SEEK_SET);
    // assert we found the list head
    if(!assert_value(fp, 0)) {
        fclose(fp);
        return NULL;
    }
    
    int32_t *page = read_int32(fp);
    fseek(fp, *page, SEEK_SET);
    // assert we found the page
    if(!assert_value(fp, 1)) {
        fclose(fp);
        return NULL;
    }

    // move on 3*4bytes
    read_int32(fp);
    if(!assert_value(fp, 0)) {
        fclose(fp);
        return NULL;
    }
    if(!assert_value(fp, 0)) {
        fclose(fp);
        return NULL;
    }

    // read in the file number, offset and 
    // length of the image strip
    *position = read_int32(fp);
    *size = read_int32(fp);
    int32_t *fileno = read_int32(fp);

    fclose(fp);
    return fileno;
}

// wipe the image data for filename, offset and length
int32_t wipe_level_data(char *filename, 
        int32_t **offset, 
        int32_t **length, 
        const char *prefix) {
    FILE *fp = fopen(filename, "r+w");

    if(fp == NULL) {
        fprintf(stderr, "Error: Could not open label file.\n");
        return -1;
    }

    if(fseek(fp, 0, SEEK_END)) {
        fprintf(stderr, "Error: Failed reading to end of file.\n");
        fclose(fp);
        return 1;
    }

    // check if offset and length are equal to file pointer at end
    bool truncation = (ftell(fp) == (**offset + **length));

    if(fseek(fp, **offset, SEEK_SET)) {
        fprintf(stderr, "Error: Failed to seek to offset.\n");
        fclose(fp);
        return -1;
    }

    char *buffer = (char *)malloc(sizeof(prefix));
    if(fread(buffer, sizeof(prefix), 1, fp) != 1) {
        fprintf(stderr, "Error: Could not read strip prefix.\n");
        fclose(fp);
        free(buffer);
        return -1;
    }
    

    // check for jpeg prefix start of file (soi)
    if(buffer[0] != prefix[0] || buffer[1] != prefix[1]) {
        fprintf(stderr, "Error: Unexpected data in file.\n");
        fclose(fp);
        free(buffer);
        return -1;
    }

    if(truncation) {
        // TODO: replace with truncation  (to make file size zero)
        // must work under win and linux!
        fseek(fp, **offset, SEEK_SET);
        char *empty_buffer = get_empty_char_buffer("0", **length, prefix);
        fwrite(empty_buffer, **length, 1, fp);
    } else {
        fseek(fp, **offset, SEEK_SET);
        char *empty_buffer = get_empty_char_buffer("0", **length, prefix);
        fwrite(empty_buffer, **length, 1, fp);
    }

    free(buffer);
    fclose(fp);
    return 0;
}

// remove label level
int32_t delete_level(char *path,
        char *index_file,
        char **data_files,
        struct mirax_layer **layers, 
        const char *layer_name,     
        const char *level_name) {
    struct mirax_level *level_to_delete = get_level_by_name(
        layers, layer_name, level_name);

    if(level_to_delete == NULL) {
        fprintf(stderr, "Error: Could not find slide barcode level.\n");
        return -1;
    }
    
    char *index_file_path = concat_path_filename(path, index_file);
    int32_t *position, *size;
    int32_t *fileno = read_data_location(
        index_file_path, level_to_delete->record, &position, &size);

    if(fileno == NULL) {
        return -1;
    }

    char *filename = concat_path_filename(path, data_files[*fileno]);

    return wipe_level_data(filename, &position, &size, JPEG_SOI);
}

// delete label record from index file
int32_t delete_record_from_index_file(char *filename, 
        int32_t record, 
        int32_t all_records) {
    FILE *fp = fopen(filename, "r+w");
    int32_t to_move = all_records - record -1;

    if(to_move == 0) {
        fclose(fp);
        return 0;
    }

    fseek(fp, MRXS_ROOT_OFFSET_NONHIER, SEEK_SET);
    int32_t *table_base = read_int32(fp);
    int32_t offset = (*table_base) + (record+1)*4;
    fseek(fp, offset, SEEK_SET);

    char buffer[to_move*4];
    if(fread(buffer, (to_move * 4), 1, fp) != 1) {
        fprintf(stderr, "Error: Unexpected length of buffer.\n");
        fclose(fp);
        return -1;
    }

    // we return to record position and overwrite 
    // the file with the tail data
    fseek(fp, (*table_base + (record * 4)), SEEK_SET);
    fwrite(&buffer, (to_move * 4), 1, fp);

    fclose(fp);
    return 0;
}

// get the predecessor of a certail level
struct mirax_level *get_next_level(struct mirax_layer **layers, 
        struct mirax_level *level) {
    // determine the layers array size
    int32_t length = sizeof(**layers) / sizeof(layers[0]);

    if(length > level->layer_id) {
        struct mirax_layer *layer = layers[level->layer_id];

        if(layer->level_count > level->id + 1) {
            return layer->levels[level->id + 1];
        }
    }

    return NULL;
}

int32_t handle_mirax(const char *filename, 
        const char *new_label_name, 
        bool unlink_directory) {
    char *path = strndup(filename, strlen(filename) - strlen(DOT_MRXS_EXT));

    struct ini_file *ini = read_slidedat_ini_file(path, SLIDEDAT);

    char *index_filename = get_value_from_ini_file(ini, HIERARCHICAL, INDEXFILE);
    char *file_count = get_value_from_ini_file(ini, DATAFILE, FILE_COUNT); 
    char *layer_count = get_value_from_ini_file(ini, HIERARCHICAL, NONHIER_COUNT);

    if(index_filename == NULL 
        || file_count == NULL 
        || layer_count == NULL) {
        fprintf(stderr, "Error: No index file specified.\n");
        return -1;
    }

    // cast file count
    int32_t f_count;
    int32_t l_count;
    sscanf(file_count, "%d", &f_count);
    sscanf(layer_count, "%d", &l_count);

    // get .dat filenames
    char **data_filenames = (char **)malloc((f_count + 1) * sizeof(char *));
    for(int32_t i = 0; i < f_count; i++) {
        char *temp_datafile_key = concat_wildcard_string_int32(FILE_, i);
        char *temp_datafile_name = get_value_from_ini_file(
            ini, DATAFILE, temp_datafile_key); 
        data_filenames[i] = temp_datafile_name;
        //printf("%i: %s\n", i, temp_datafile_name);
    }

    struct mirax_file *mirax_file = get_mirax_file_structure(ini, l_count);

    // wipe the image data in the data file
    int32_t result = delete_level(path, 
        index_filename, 
        data_filenames, 
        mirax_file->layers, 
        SCAN_DATA_LAYER, 
        SLIDE_BARCODE);

    if(result != 0) {
        return result;
    }

    // unlink directory
    if(unlink_directory) {
        struct mirax_level *level_to_delete = get_level_by_name(
            mirax_file->layers, SCAN_DATA_LAYER, SLIDE_BARCODE);

        // delete record in index dat
        char *full_index_filename = concat_path_filename(path, index_filename);
        result = delete_record_from_index_file(
            full_index_filename, level_to_delete->record, mirax_file->all_records_count);

        if(result != 0) {
            return result;
        }

        // modify mirax layers
        result = delete_group_form_ini_file(ini, level_to_delete->section);

        if(result != 0) {
            return result;
        }

        struct mirax_level *current_level = level_to_delete;
        struct mirax_level *next_level = get_next_level(
            mirax_file->layers, current_level);

        while(next_level != NULL) {
            // we determine the predecessor level of 
            // the deleted level and change non hier entries
            rename_section_name_for_level_in_section(
                ini, HIERARCHICAL, current_level, next_level);
            // go to next level in hierarchy
            *current_level = *next_level;
            next_level = get_next_level(mirax_file->layers, current_level);
        }

        // change count in hierarchy layer
        decrement_value_for_group_and_key(ini, HIERARCHICAL, "NONHIER_0_COUNT");
        // we need to delete old unused entries now
        char *nonhier_plain = concat_wildcard_string_m_int32(
            NONHIER_X_VAL_X, 0, level_to_delete->id);
        char *nonhier_section = concat_wildcard_string_m_int32(
            NONHIER_X_VAL_X_SECTION, 0, level_to_delete->id);
        char *nonhier_imgx = concat_wildcard_string_m_int32(
            NONHIER_X_VAL_X_IMAGENUMBER_X, 0, level_to_delete->id);
        char *nonhier_imgy = concat_wildcard_string_m_int32(
            NONHIER_X_VAL_X_IMAGENUMBER_Y, 0, level_to_delete->id);
        remove_entry_for_group_and_key(ini, HIERARCHICAL, nonhier_plain);
        remove_entry_for_group_and_key(ini, HIERARCHICAL, nonhier_section);
        remove_entry_for_group_and_key(ini, HIERARCHICAL, nonhier_imgx);
        remove_entry_for_group_and_key(ini, HIERARCHICAL, nonhier_imgy);

        set_value_for_group_and_key(ini, GENERAL, SLIDE_NAME, new_label_name);
        char *project_name = concat_wildcard_string_string(PROJECT_X, new_label_name);
        set_value_for_group_and_key(ini, GENERAL, PROJECT_NAME, project_name);

        if(write_ini_file(ini, path, SLIDEDAT) == -1) {
            return -1;
        }
    }

    return 0;
}

int is_mirax(const char *filename) {
    const char *ext = get_filename_ext(filename);

    if(strcmp(ext, MRXS_EXT) == 0) {
        return 1;
    } else {
        return 0;
    }  
}