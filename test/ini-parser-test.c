#include "CUnit/Basic.h"

#include "../src/ini-parser.h"

// ####################### functions to test ####################### //

struct ini_file *read_slidedat_ini_file(const char *path, 
        const char *ini_filename);

char *get_value_from_ini_file(struct ini_file *ini_file, 
        const char *group, 
        const char *entry_key);

int32_t get_all_records(struct ini_file *ini_file);

int32_t delete_group_form_ini_file(struct ini_file *ini_file, 
        char *group_name);

void rename_section_name_for_level_in_section(struct ini_file *ini_file, 
        const char *group_name, 
        struct mirax_level *current_level, 
        struct mirax_level *next_level);

void set_value_for_group_and_key(struct ini_file *ini_file, 
        const char *group_name, 
        const char *key, 
        const char *value);

void remove_entry_for_group_and_key(struct ini_file *ini_file, 
        const char *group_name, 
        const char *key);

void decrement_value_for_group_and_key(struct ini_file *ini_file, 
        const char *group_name, 
        const char *key);

int32_t write_ini_file(struct ini_file *ini_file, 
        const char *path, 
        const char *filename);

// ####################### test cases ####################### //

void test_first() {
        // TODO
        CU_ASSERT_TRUE(true);
}

// ####################### test case setup ####################### //

CU_TestInfo testcases3[] = {
        {"Test [first] 1:", test_first},

        CU_TEST_INFO_NULL
};

CU_SuiteInfo suites2[] = {
        {"Testing ini-parser.c:", NULL, NULL, NULL, NULL, testcases3},
        CU_SUITE_INFO_NULL
};

void AddTestsIniParser(void)
{
        assert(NULL != CU_get_registry());
        assert(!CU_is_test_running());

        if(CUE_SUCCESS != CU_register_suites(suites2)){
                fprintf(stderr, "Register suites failed - %s ", CU_get_error_msg());
                exit(1);
        }
}