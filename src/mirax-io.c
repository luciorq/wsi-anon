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
static const char NONHIER_X_VAL_X_SECTION[] = "NONHIER_%d_VAL_%d_SECTION";
static const char NONHIER_X_VAL_X_IMAGENUMBER_X[] = "NONHIER_%d_VAL_%d_IMAGENUMBER_X";
static const char NONHIER_X_VAL_X_IMAGENUMBER_Y[] = "NONHIER_%d_VAL_%d_IMAGENUMBER_Y";
static const char _SECTION[] = "_SECTION";
static const char FILE_[] = "FILE_%d";
static const char PROJECT_X[] = "Project (%s)";

// define layer and level name in mirax file
static const char *SCAN_DATA_LAYER = "Scan data layer";
// slide label
static const char *SLIDE_BARCODE = "ScanDataLayer_SlideBarcode";
// macro image
static const char *SLIDE_THUMBNAIL = "ScanDataLayer_SlideThumbnail";

const char *concat_wildcard_string_int32(const char *str, int32_t integer) {
    char *result_string = (char *)malloc(strlen(str) + number_of_digits(integer) + 1);
    sprintf(result_string, str, integer);
    return result_string;
}

const char *concat_wildcard_string_m_int32(const char *str, int32_t integer1, int32_t integer2) {
    char *result_string =
        (char *)malloc(strlen(str) + number_of_digits(integer1) + number_of_digits(integer2) + 1);
    sprintf(result_string, str, integer1, integer2);
    return result_string;
}

const char *concat_wildcard_string_string(const char *wildcard_str, const char *replacement) {
    char *result_string = (char *)malloc(strlen(wildcard_str) + strlen(replacement) + 1);
    sprintf(result_string, wildcard_str, replacement);
    return result_string;
}

// retrieve the file structure of the mirax file from the
// Slidedat.ini file
struct mirax_file *get_mirax_file_structure(struct ini_file *ini, int32_t l_count) {
    // initialize mirrax file and array of associated layers
    struct mirax_file *mirax_file = (struct mirax_file *)malloc(sizeof(struct mirax_file));

    struct mirax_layer **layers =
        (struct mirax_layer **)malloc(l_count * sizeof(struct mirax_layer));

    int32_t records = 0;
    for (int32_t layer_id = 0; layer_id < l_count; layer_id++) {
        // initialize and populate new layer
        struct mirax_layer *layer = (struct mirax_layer *)malloc(sizeof(struct mirax_layer));

        const char *nonhier_level_count_key =
            concat_wildcard_string_int32(NONHIER_X_COUNT, layer_id);

        const char *nonhier_level_count =
            get_value_from_ini_file(ini, HIERARCHICAL, nonhier_level_count_key);

        int32_t nhl_count;
        sscanf(nonhier_level_count, "%d", &nhl_count);
        layer->level_count = nhl_count;

        const char *layer_name_key = concat_wildcard_string_int32(NONHIER_X_NAME, layer_id);

        layer->layer_name = get_value_from_ini_file(ini, HIERARCHICAL, layer_name_key);

        // initialize and allocate new array of sub lavels of array
        struct mirax_level **levels =
            (struct mirax_level **)malloc(nhl_count * sizeof(struct mirax_level));

        for (int32_t level_id = 0; level_id < nhl_count; level_id++) {
            // initialize and populate sublevels
            // printf("layer %i, level %i\n", layer_id, level_id);
            struct mirax_level *level = (struct mirax_level *)malloc(sizeof(struct mirax_level));

            level->id = level_id;
            level->layer_id = layer_id;
            level->record = layer_id + level_id;
            const char *key_prefix =
                concat_wildcard_string_m_int32(NONHIER_X_VAL_X, layer_id, level_id);
            level->key_prefix = key_prefix;
            level->name = get_value_from_ini_file(ini, HIERARCHICAL, level->key_prefix);
            int32_t length = strlen(NONHIER_X_VAL_X) + number_of_digits(layer_id) +
                             number_of_digits(level_id) + 1;
            char *section_key = (char *)malloc(length + sizeof(_SECTION) + 2);
            strcpy(section_key, level->key_prefix);
            strcat(section_key, _SECTION);
            level->section_key = section_key;
            level->section = get_value_from_ini_file(ini, HIERARCHICAL, level->section_key);
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
struct mirax_level *get_level_by_name(struct mirax_layer **layers, const char *layer_name,
                                      const char *level_name) {
    int32_t length = sizeof(**layers) / sizeof(layers[0]);
    for (int32_t i = 0; i < length; i++) {
        struct mirax_layer *layer = layers[i];
        // printf("layer name %s\n", layer->layer_name);
        if (strcmp(layer->layer_name, layer_name) == 0) {
            for (int j = 0; j < layer->level_count; j++) {
                struct mirax_level *level = layer->levels[j];
                // printf("level name %s\n", level->name);
                if (strcmp(level->name, level_name) == 0) {
                    return level;
                }
            }
        }
    }
    return NULL;
}

// read a signed integer 32 from file stream
int32_t *read_int32(file_t *fp) {
    int32_t *buffer = (int32_t *)malloc(sizeof(int32_t));

    if (file_read(buffer, sizeof(*buffer), 1, fp) != 1) {
        free(buffer);
        return NULL;
    }

    return buffer;
}

// assert a int32_t has a certain value
// used to skip pointer position on stream
bool assert_value(file_t *fp, int32_t value) {
    int32_t *v_to_check = read_int32(fp);
    return *v_to_check == value;
}

// read file number, position and size frrom index dat
int32_t *read_data_location(const char *filename, int32_t record, int32_t **position,
                            int32_t **size) {
    file_t *fp = file_open(filename, "r");

    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open file stream.\n");
        return NULL;
    }

    if (file_seek(fp, MRXS_ROOT_OFFSET_NONHIER, SEEK_SET)) {
        fprintf(stderr, "Error: Can not seek to mrxs offset in Index.dat.\n");
        file_close(fp);
        return NULL;
    }

    int32_t *table_base = read_int32(fp);
    file_seek(fp, (*table_base + (record * 4)), SEEK_SET);

    int32_t *list_header = read_int32(fp);
    file_seek(fp, *list_header, SEEK_SET);
    // assert we found the list head
    if (!assert_value(fp, 0)) {
        file_close(fp);
        return NULL;
    }

    int32_t *page = read_int32(fp);
    file_seek(fp, *page, SEEK_SET);
    // assert we found the page
    if (!assert_value(fp, 1)) {
        file_close(fp);
        return NULL;
    }

    // move on 3*4bytes
    read_int32(fp);
    if (!assert_value(fp, 0)) {
        file_close(fp);
        return NULL;
    }
    if (!assert_value(fp, 0)) {
        file_close(fp);
        return NULL;
    }

    // read in the file number, offset and
    // length of the image strip
    *position = read_int32(fp);
    *size = read_int32(fp);
    int32_t *fileno = read_int32(fp);

    file_close(fp);
    return fileno;
}

// wipe the image data for filename, offset and length
int32_t wipe_level_data(const char *filename, int32_t **offset, int32_t **length,
                        const char *prefix, const char *suffix) {
    file_t *fp = file_open(filename, "r+w");

    if (fp == NULL) {
        fprintf(stderr, "Error: Could not open label file.\n");
        return -1;
    }

    if (file_seek(fp, **offset, SEEK_SET)) {
        fprintf(stderr, "Error: Failed to seek to offset.\n");
        file_close(fp);
        return -1;
    }

    char *buffer = (char *)malloc(sizeof(prefix));
    if (file_read(buffer, sizeof(prefix), 1, fp) != 1) {
        fprintf(stderr, "Error: Could not read strip prefix.\n");
        file_close(fp);
        free(buffer);
        return -1;
    }

    // check for jpeg prefix start of file (soi)
    if (buffer[0] != prefix[0] || buffer[1] != prefix[1]) {
        fprintf(stderr, "Error: Unexpected data in file.\n");
        file_close(fp);
        free(buffer);
        return -1;
    }

    // write empty jpeg image to file
    const char *empty_buffer = get_empty_char_buffer("0", **length, prefix, suffix);
    file_seek(fp, **offset, SEEK_SET);
    file_write(empty_buffer, **length, 1, fp);

    free(buffer);
    file_close(fp);
    return 0;
}

// remove label level
int32_t delete_level(const char *path, const char *index_file, const char **data_files,
                     struct mirax_layer **layers, const char *layer_name, const char *level_name) {
    struct mirax_level *level_to_delete = get_level_by_name(layers, layer_name, level_name);

    if (level_to_delete == NULL) {
        fprintf(stderr, "Error: Could not find expected level.\n");
        return -1;
    }

    const char *index_file_path = concat_path_filename(path, index_file);
    int32_t *position, *size;
    int32_t *fileno =
        read_data_location(index_file_path, level_to_delete->record, &position, &size);

    if (fileno == NULL) {
        return -1;
    }

    const char *filename = concat_path_filename(path, data_files[*fileno]);

    return wipe_level_data(filename, &position, &size, JPEG_SOI, JPEG_EOI);
}

// delete label record from index file
int32_t delete_record_from_index_file(const char *filename, int32_t record, int32_t all_records) {
    file_t *fp = file_open(filename, "r+w");
    int32_t to_move = all_records - record - 1;

    if (to_move == 0) {
        file_close(fp);
        return 0;
    }

    file_seek(fp, MRXS_ROOT_OFFSET_NONHIER, SEEK_SET);
    int32_t *table_base = read_int32(fp);
    int32_t offset = (*table_base) + (record + 1) * 4;
    file_seek(fp, offset, SEEK_SET);

    char buffer[to_move * 4];
    if (file_read(buffer, (to_move * 4), 1, fp) != 1) {
        fprintf(stderr, "Error: Unexpected length of buffer.\n");
        file_close(fp);
        return -1;
    }

    // we return to record position and overwrite
    // the file with the tail data
    file_seek(fp, (*table_base + (record * 4)), SEEK_SET);
    file_write(&buffer, (to_move * 4), 1, fp);

    file_close(fp);
    return 0;
}

// get the predecessor of a certain level
struct mirax_level *get_next_level(struct mirax_layer **layers, struct mirax_level *level) {
    struct mirax_layer *layer = layers[level->layer_id];

    for (int i = 0; i < layer->level_count; i++) {
        if (layer->levels[i]->id == (level->id + 1)) {
            return layer->levels[i];
        }
    }

    return NULL;
}

// TODO: CHECK IF THIS WORKS UNDER WINDOWS!!
// duplicate and rename qthe mirax file file.mrxs and
// the associated folder with the image data
// return new path name of image data folder
// filename will be modified to new filename
const char *duplicate_mirax_filedata(const char *filename, const char *new_label_name,
                                     const char *file_extension) {
    // retrive filename from whole file path
    const char *_filename = get_filename_from_path(filename);

    if (_filename == NULL) {
        fprintf(stderr, "Error: Could not retrieve filename from filepath.\n");
        return NULL;
    }

    // get the directory path
    int32_t diff = strlen(filename) - strlen(_filename);
    char path[diff + 1];
    memcpy(path, &filename[0], diff);
    path[diff] = '\0';

    int32_t diff2 = strlen(_filename) - strlen(file_extension);
    char old_filename_wo_ext[diff2 + 1];
    memcpy(old_filename_wo_ext, &_filename[0], diff2);
    old_filename_wo_ext[diff2] = '\0';

    // now we can concat the new filename
    if (new_label_name == NULL) {
        // if no label is given, we give the file a generic name
        char dummy_prefix[] = "ANONYMIZED_";
        char *temp_label_name =
            (char *)malloc(strlen(dummy_prefix) + strlen(old_filename_wo_ext) + 1);
        strcpy(temp_label_name, dummy_prefix);
        strcat(temp_label_name, old_filename_wo_ext);
        // concat string
        new_label_name = temp_label_name;
    }
    const char *new_filename = concat_path_filename_ext(path, new_label_name, file_extension);

    if (file_exists(new_filename)) {
        fprintf(stderr, "Error: File with stated filename [%s] already exists.\
            Remove file or set label name.\n",
                new_filename);
        return NULL;
    }

    int32_t copy_result = copy_file_v2(filename, new_filename);
    // we copy the file in our current directory
    if (copy_result != 0) {
        return NULL;
    }

    filename = new_filename;

    // copy mirax directory
    char *new_path = (char *)malloc(strlen(path) + strlen(new_label_name) + 1);
    strcpy(new_path, path);
    strcat(new_path, new_label_name);

    char *old_path = (char *)malloc(strlen(path) + strlen(old_filename_wo_ext) + 1);
    strcpy(old_path, path);
    strcat(old_path, old_filename_wo_ext);

    int32_t result = copy_directory(old_path, new_path);
    free(old_path);

    if (result != -1) {
        return new_path;
    } else {
        return NULL;
    }
}

// remove a level entry by id from mirax layer structure
struct mirax_layer *delete_level_by_id(struct mirax_layer *layer, int32_t level_id) {
    struct mirax_layer *temp = (struct mirax_layer *)malloc(sizeof(struct mirax_layer));
    temp->layer_name = layer->layer_name;
    temp->level_count = layer->level_count - 1;

    // new array with size one less than old array
    struct mirax_level **temp_levels =
        (struct mirax_level **)malloc(temp->level_count * sizeof(struct mirax_level));

    // copy all elements before the index
    if (level_id != 0) {
        memcpy(temp_levels, layer->levels, level_id * sizeof(struct mirax_level));
    }

    // copy all elements after the index
    if (level_id != (layer->level_count - 1)) {
        memcpy(temp_levels + level_id, layer->levels + level_id + 1,
               (temp->level_count - level_id) * sizeof(struct mirax_level));
    }

    temp->levels = temp_levels;

    free(layer);
    return temp;
}

int32_t delete_level_from_mirax_file(struct mirax_file *mirax_file,
                                     struct mirax_level *level_to_delete) {
    // find the level id that we want to delete
    for (int i = 0; i < mirax_file->count_layers; i++) {
        int32_t level_id = -1;
        struct mirax_layer *layer = mirax_file->layers[i];
        for (int j = 0; j < layer->level_count; j++) {
            struct mirax_level *level = layer->levels[j];
            if (level_to_delete->id == level->id) {
                level_id = j;
                break;
            }
        }

        // remove the level entry and exchange the mirax layer in the array
        struct mirax_layer *new_layer = (struct mirax_layer *)malloc(sizeof(struct mirax_layer));
        new_layer = delete_level_by_id(layer, level_id);

        if (new_layer != NULL) {
            mirax_file->layers[i] = new_layer;
            mirax_file->all_records_count--;
            return 0;
        }
    }
    return -1;
}

void rename_mirax_file_levels(struct mirax_file *mirax_file, int32_t layer,
                              struct mirax_level *current_level, struct mirax_level *next_level) {
    char *name_copy = (char *)malloc(strlen(next_level->name) + 1);
    strcpy(name_copy, next_level->name);
    mirax_file->layers[layer]->levels[current_level->id]->name = name_copy;
    char *section_copy = (char *)malloc(strlen(next_level->name) + 1);
    strcpy(section_copy, next_level->name);
    mirax_file->layers[layer]->levels[current_level->id]->section = section_copy;
}

void delete_last_entry_from_ini_file(struct ini_file *ini, struct mirax_file *mirax_file) {
    // change count in hierarchy layer
    decrement_value_for_group_and_key(ini, HIERARCHICAL, "NONHIER_0_COUNT");

    int32_t id_to_delete = mirax_file->layers[0]->level_count;

    // we need to delete all old unused entries now
    const char *nonhier_plain = concat_wildcard_string_m_int32(NONHIER_X_VAL_X, 0, id_to_delete);
    const char *nonhier_section =
        concat_wildcard_string_m_int32(NONHIER_X_VAL_X_SECTION, 0, id_to_delete);
    const char *nonhier_imgx =
        concat_wildcard_string_m_int32(NONHIER_X_VAL_X_IMAGENUMBER_X, 0, id_to_delete);
    const char *nonhier_imgy =
        concat_wildcard_string_m_int32(NONHIER_X_VAL_X_IMAGENUMBER_Y, 0, id_to_delete);
    remove_entry_for_group_and_key(ini, HIERARCHICAL, nonhier_plain);
    remove_entry_for_group_and_key(ini, HIERARCHICAL, nonhier_section);
    remove_entry_for_group_and_key(ini, HIERARCHICAL, nonhier_imgx);
    remove_entry_for_group_and_key(ini, HIERARCHICAL, nonhier_imgy);
    mirax_file->layers[0]->level_count--;
}

void unlink_macro_and_label_image(struct ini_file *ini, struct mirax_level *macro,
                                  struct mirax_level *label, struct mirax_file *mirax_file) {
    struct mirax_level *current_level = macro;
    struct mirax_level *next_level = get_next_level(mirax_file->layers, label);

    while (next_level != NULL) {
        // we determine the predecessor level of
        // the deleted level and change non hier entries
        rename_section_name_for_level_in_section(ini, HIERARCHICAL, current_level, next_level);
        // when deleting label and macro image, we need to skip
        // two hierarchical positions each iteration
        current_level = get_next_level(mirax_file->layers, current_level);
        next_level = get_next_level(mirax_file->layers, next_level);
    }

    // delete last level, that is now obsolete
    delete_level_from_mirax_file(mirax_file, current_level);
    free(current_level);

    delete_last_entry_from_ini_file(ini, mirax_file);
    delete_last_entry_from_ini_file(ini, mirax_file);
}

void unlink_level(struct ini_file *ini, struct mirax_level *level_to_delete,
                  struct mirax_file *mirax_file) {
    // modify internal mirax representation
    struct mirax_level *current_level = level_to_delete;
    struct mirax_level *next_level = get_next_level(mirax_file->layers, current_level);
    while (next_level != NULL) {
        // we determine the predecessor level of
        // the deleted level and change non hier entries
        rename_section_name_for_level_in_section(ini, HIERARCHICAL, current_level, next_level);
        // rename_mirax_file_levels(mirax_file, 0, current_level, next_level);
        // go to next level in hierarchy
        *current_level = *next_level;
        next_level = get_next_level(mirax_file->layers, current_level);
    }
    // delete last level, that is now obsolete
    delete_level_from_mirax_file(mirax_file, current_level);
    free(current_level);

    delete_last_entry_from_ini_file(ini, mirax_file);
}

int32_t wipe_data_in_index_file(const char *path, const char *index_filename,
                                struct mirax_level *level_to_delete,
                                struct mirax_file *mirax_file) {
    int32_t result = -1;
    // delete record in index dat
    const char *full_index_filename = concat_path_filename(path, index_filename);
    result = delete_record_from_index_file(full_index_filename, level_to_delete->record,
                                           mirax_file->all_records_count);
    return result;
}

int32_t handle_mirax(const char **filename, const char *new_label_name, bool keep_macro_image,
                     bool disable_unlinking, bool do_inplace) {
    fprintf(stdout, "Anonymize Mirax WSI...\n");
    const char *path = strndup(*filename, strlen(*filename) - strlen(DOT_MRXS_EXT));

    if (!do_inplace) {
        path = duplicate_mirax_filedata(*filename, new_label_name, DOT_MRXS_EXT);

        if (path == NULL || filename == NULL) {
            fprintf(stderr, "Error: Failed to copy mirax files.\n");
            return -1;
        }
        *filename = path;
    }

    struct ini_file *ini = read_slidedat_ini_file(path, SLIDEDAT);
    const char *index_filename = get_value_from_ini_file(ini, HIERARCHICAL, INDEXFILE);
    const char *file_count = get_value_from_ini_file(ini, DATAFILE, FILE_COUNT);
    const char *layer_count = get_value_from_ini_file(ini, HIERARCHICAL, NONHIER_COUNT);

    if (index_filename == NULL || file_count == NULL || layer_count == NULL) {
        fprintf(stderr, "Error: No index file specified.\n");
        return -1;
    }

    // cast file count
    int32_t f_count;
    int32_t l_count;
    sscanf(file_count, "%d", &f_count);
    sscanf(layer_count, "%d", &l_count);

    // get .dat filenames
    const char **data_filenames = (const char **)malloc((f_count + 1) * sizeof(char *));
    for (int32_t i = 0; i < f_count; i++) {
        const char *temp_datafile_key = concat_wildcard_string_int32(FILE_, i);
        const char *temp_datafile_name = get_value_from_ini_file(ini, DATAFILE, temp_datafile_key);
        data_filenames[i] = temp_datafile_name;
        // printf("%i: %s\n", i, temp_datafile_name);
    }

    struct mirax_file *mirax_file = get_mirax_file_structure(ini, l_count);

    // wipe the image data in the data file
    // slide label
    int32_t result = delete_level(path, index_filename, data_filenames, mirax_file->layers,
                                  SCAN_DATA_LAYER, SLIDE_BARCODE);

    // delete macro image
    if (!keep_macro_image) {
        result = delete_level(path, index_filename, data_filenames, mirax_file->layers,
                              SCAN_DATA_LAYER, SLIDE_THUMBNAIL);
    }

    // unlink directory
    if (!disable_unlinking) {
        // slide label
        struct mirax_level *slide_label =
            get_level_by_name(mirax_file->layers, SCAN_DATA_LAYER, SLIDE_BARCODE);

        if (slide_label != NULL) {
            if (wipe_data_in_index_file(path, index_filename, slide_label, mirax_file) != 0) {
                fprintf(stderr, "Error: Failed to wipe data in Index.dat.\n");
                return -1;
            }

            if (delete_group_form_ini_file(ini, slide_label->section) != 0) {
                fprintf(stderr, "Error: Failed to delete group in Slidedat.ini.\n");
                return -1;
            }
        }

        // unlink macro image
        if (!keep_macro_image) {
            struct mirax_level *macro_image =
                get_level_by_name(mirax_file->layers, SCAN_DATA_LAYER, SLIDE_THUMBNAIL);

            if (macro_image != NULL) {
                if (wipe_data_in_index_file(path, index_filename, macro_image, mirax_file) != 0) {
                    fprintf(stderr, "Error: Failed to wipe data in Index.dat.\n");
                    return -1;
                }

                if (delete_group_form_ini_file(ini, macro_image->section) != 0) {
                    fprintf(stderr, "Error: Failed to delete group in Slidedat.ini.\n");
                    return -1;
                }

                if (slide_label != NULL) {
                    // unlink macro AND label image
                    unlink_macro_and_label_image(ini, macro_image, slide_label, mirax_file);
                } else {
                    // unlink only macro image
                    unlink_level(ini, macro_image, mirax_file);
                }
            }
        } else {
            if (slide_label != NULL) {
                // unlink only label image
                unlink_level(ini, slide_label, mirax_file);
            }
        }

        // general data in slidedat ini
        set_value_for_group_and_key(ini, GENERAL, SLIDE_NAME, new_label_name);
        const char *project_name = concat_wildcard_string_string(PROJECT_X, new_label_name);
        set_value_for_group_and_key(ini, GENERAL, PROJECT_NAME, project_name);

        if (write_ini_file(ini, path, SLIDEDAT) == -1) {
            return -1;
        }
    }

    free(mirax_file);

    return result;
}

int is_mirax(const char *filename) {
    const char *ext = get_filename_ext(filename);

    if (strcmp(ext, MRXS_EXT) == 0) {
        return 1;
    } else {
        return 0;
    }
}