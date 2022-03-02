#include "CUnit/Basic.h"

#include "../../src/ini-parser.h"

// ####################### functions to test ####################### //

struct ini_file *read_slidedat_ini_file(const char *path, const char *ini_filename);

const char *get_value_from_ini_file(struct ini_file *ini_file, const char *group,
                                    const char *entry_key);

int32_t delete_group_form_ini_file(struct ini_file *ini_file, const char *group_name);

void rename_section_name_for_level_in_section(struct ini_file *ini_file, const char *group_name,
                                              struct mirax_level *current_level,
                                              struct mirax_level *next_level);

void set_value_for_group_and_key(struct ini_file *ini_file, const char *group_name, const char *key,
                                 const char *value);

void remove_entry_for_group_and_key(struct ini_file *ini_file, const char *group_name,
                                    const char *key);

void decrement_value_for_group_and_key(struct ini_file *ini_file, const char *group_name,
                                       const char *key);

int32_t write_ini_file(struct ini_file *ini_file, const char *path, const char *filename);

// ####################### test cases ####################### //

struct ini_file *mock_ini_file() {
    struct ini_file *ini_file = (struct ini_file *)malloc(sizeof(struct ini_file));
    struct ini_group *groups = (struct ini_group *)malloc(sizeof(struct ini_group));
    struct ini_group *group = (struct ini_group *)malloc(sizeof(struct ini_group));
    struct ini_entry *entries = (struct ini_entry *)malloc(3 * sizeof(struct ini_entry));
    struct ini_entry *entry1 = (struct ini_entry *)malloc(sizeof(struct ini_entry));
    struct ini_entry *entry2 = (struct ini_entry *)malloc(sizeof(struct ini_entry));
    struct ini_entry *entry3 = (struct ini_entry *)malloc(sizeof(struct ini_entry));

    entry1->key = "TestKey1";
    entry1->value = "TestValue1";
    entry2->key = "TestKey2";
    entry2->value = "TestValue2";
    entry3->key = "TestKey3";
    entry3->value = "1000";
    entries[0] = *entry1;
    entries[1] = *entry2;
    entries[2] = *entry3;
    group->group_identifier = "Identifier";
    group->entry_count = 3;
    group->start_line = 0;
    group->entries = entries;
    ini_file->group_count = 1;
    ini_file->groups = group;
    return ini_file;
}

void test_get_value_from_ini_file() {
    struct ini_file *ini_file = mock_ini_file();
    const char *result = get_value_from_ini_file(ini_file, "Identifier", "TestKey2");
    CU_ASSERT_PTR_NOT_NULL(result);
    CU_ASSERT_STRING_EQUAL(result, "TestValue2");
    free(ini_file);
}

void test_delete_group_from_ini_file() {
    struct ini_file *ini_file = mock_ini_file();
    int32_t result = delete_group_form_ini_file(ini_file, "Identifier");
    CU_ASSERT_EQUAL(result, 0);
    CU_ASSERT_EQUAL(ini_file->group_count, 0);
    CU_ASSERT_EQUAL(ini_file->groups[0].entry_count, 0);
}

void test_set_value_for_group_and_key() {
    struct ini_file *ini_file = mock_ini_file();
    set_value_for_group_and_key(ini_file, "Identifier", "TestKey1", "NEW_VALUE");
    CU_ASSERT_STRING_EQUAL(ini_file->groups[0].entries[0].value, "NEW_VALUE");
}

void test_remove_entry_for_group_and_key() {
    struct ini_file *ini_file = mock_ini_file();
    remove_entry_for_group_and_key(ini_file, "Identifier", "TestKey1");
    CU_ASSERT_EQUAL(ini_file->groups[0].entry_count, 2);
    CU_ASSERT_STRING_EQUAL(ini_file->groups[0].entries[0].key, "TestKey2");
}

void test_decrement_value_for_group_and_key1() {
    struct ini_file *ini_file = mock_ini_file();
    decrement_value_for_group_and_key(ini_file, "Identifier", "TestKey3");
    CU_ASSERT_STRING_EQUAL(ini_file->groups[0].entries[2].value, "999");
}

void test_decrement_value_for_group_and_key2() {
    struct ini_file *ini_file = mock_ini_file();
    decrement_value_for_group_and_key(ini_file, "Identifier", "TestKey2");
    CU_ASSERT_STRING_NOT_EQUAL(ini_file->groups[0].entries[1].value, "TestValue2");
}

void test_write_ini_file() {
    // TODO
    CU_ASSERT_TRUE(true);
}

// ####################### test case setup ####################### //

CU_TestInfo testcases3[] = {

    {"Test [get_value_from_ini_file]:", test_get_value_from_ini_file},
    {"Test [delete_group_from_ini_file]:", test_delete_group_from_ini_file},
    {"Test [set_value_for_group_and_key]:", test_set_value_for_group_and_key},
    {"Test [remove_entry_for_group_and_key]:", test_remove_entry_for_group_and_key},
    {"Test [decrement_value_for_group_and_key] 1:", test_decrement_value_for_group_and_key1},
    {"Test [decrement_value_for_group_and_key] 2:", test_decrement_value_for_group_and_key2},
    {"Test [write_ini_file]:", test_write_ini_file},
    CU_TEST_INFO_NULL};

CU_SuiteInfo suites2[] = {{"Testing ini-parser.c:", NULL, NULL, NULL, NULL, testcases3},
                          CU_SUITE_INFO_NULL};

void AddTestsIniParser(void) {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if (CUE_SUCCESS != CU_register_suites(suites2)) {
        fprintf(stderr, "Register suites failed - %s ", CU_get_error_msg());
        exit(1);
    }
}