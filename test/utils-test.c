#include "CUnit/Basic.h"

#include "../src/utils.h"

// ####################### functions to test ####################### //

extern char **str_split(char* a_str, const char a_delim);

extern const char *get_filename_ext(const char *filename);

extern char *get_empty_char_buffer(const char *x, 
    uint64_t length, 
    const char *prefix);

extern bool starts_with(char *str, const char *pre);

extern char *get_string_between_delimiters(char *buffer, 
    const char *delimiter1, 
    const char *delimiter2);

extern void remove_leading_spaces(char *str);

extern char *concat_path_filename(const char *path, const char *filename);

extern char *concat_path_filename_ext(const char *path, 
    const char *filename,
    const char *ext);

extern char *get_filename_from_path(char *path);

extern char *int32_to_str(int32_t integer);

extern int32_t number_of_digits(int32_t integer);

extern char *add_square_brackets(const char *str);

extern char *add_equals_sign(const char *str1, const char *str2);

extern bool contains(const char *str1, const char *str2);

extern uint16_t _swap_uint16(uint16_t value);

extern uint32_t _swap_uint32(uint32_t value);

extern uint64_t _swap_uint64(uint64_t value);

// ####################### test cases ####################### //

void test_str_split1() {
    char *input = strdup("strA;strB");
    char **result = str_split(input, ';');
    CU_ASSERT_STRING_EQUAL(result[0], "strA");
    CU_ASSERT_STRING_EQUAL(result[1], "strB");
}

void test_str_split2() {
    char *input = strdup("strA;strB;strC;");
    char **result = str_split(input, ';');
    CU_ASSERT_STRING_EQUAL(result[0], "strA");
    CU_ASSERT_STRING_EQUAL(result[1], "strB");
    CU_ASSERT_STRING_EQUAL(result[2], "strC");
}

void test_str_split3() {
    char *input = strdup("strA-strB");
    char **result = str_split(input, ';');
    CU_ASSERT_STRING_EQUAL(result[0], "strA-strB");
}

void test_get_filename_ext1() {
    const char *input = strdup("C:\\test_dir\\filename.ext123");
    const char *result = get_filename_ext(input);
    CU_ASSERT_STRING_EQUAL(result, "ext123");
}

void test_get_filename_ext2() {
    const char *input = strdup("C:\\test_dir\\filename");
    const char *result = get_filename_ext(input);
    CU_ASSERT_STRING_EQUAL(result, "");
}

void test_get_empty_char_buffer() {
    const char *prefix = strdup("\200");
    const char *x_char = strdup("0");
    const char *result = get_empty_char_buffer(x_char, 5, prefix);
    CU_ASSERT_STRING_EQUAL(result, "\20000000");
}

void test_starts_with1() {
    char *str = strdup("\200ab45ie23vf40");
    bool result = starts_with(str, "\200");
    CU_ASSERT_TRUE(result);
}

void test_starts_with2() {
    char *str = strdup("C:\\path\\to\\somewhere");
    const char *prefix = strdup("C:\\path\\to");
    bool result = starts_with(str, prefix);
    CU_ASSERT_TRUE(result);
}

void test_get_string_between_delimiters1() {
    char *str = strdup("Extract >that< string!");
    char *result = get_string_between_delimiters(str, ">", "<");
    CU_ASSERT_STRING_EQUAL(result, "that");
}

void test_get_string_between_delimiters2() {
    char *str = strdup("Extract >>that<<  >s<tring!");
    char *result = get_string_between_delimiters(str, ">", "<");
    CU_ASSERT_STRING_EQUAL(result, ">that");
}

void test_remove_leading_spaces() {
    char *str = strdup("         removes only   leading   whitespaces");
    remove_leading_spaces(str);
    CU_ASSERT_STRING_EQUAL(str, "removes only   leading   whitespaces");
}

void test_concat_path_filename() {
    const char *path = strdup("C:/path/to");
    const char *filename = strdup("filename.ext");
    char *result = concat_path_filename(path, filename);
    CU_ASSERT_STRING_EQUAL(result, "C:/path/to/filename.ext");
}

void test_get_filename_from_path() {
    char *path = strdup("C:\\path\\to\\filename.ext");
    char *result = get_filename_from_path(path);
    CU_ASSERT_STRING_EQUAL(result, "filename.ext");
}

void test_int32_to_str() {
    char *result = int32_to_str(INT32_MIN);
    CU_ASSERT_STRING_EQUAL(result, "-2147483648");
    char *result2 = int32_to_str(INT32_MAX);
    CU_ASSERT_STRING_EQUAL(result2, "2147483647");
}

void test_number_of_digits() {
    int32_t result = number_of_digits(INT32_MIN);
    CU_ASSERT_EQUAL(result, 11);
    int32_t result2 = number_of_digits(INT32_MAX);
    CU_ASSERT_EQUAL(result2, 10);
}

void test_add_square_brackets() {
    char *input = strdup("teststring");
    char *result = add_square_brackets(input);
    CU_ASSERT_STRING_EQUAL(result, "[teststring]");
}

void test_add_equals_sign() {
    char *input1 = strdup("teststring1");
    char *input2 = strdup("teststring2");
    char *result = add_equals_sign(input1, input2);
    CU_ASSERT_STRING_EQUAL(result, "teststring1 = teststring2");
}

void test_contains1() {
    char *input1 = strdup("testst&/Djs()ring1");
    char *c = strdup("&/Djs()");
    bool result = contains(input1, c);
    CU_ASSERT_TRUE(result);
}

void test_contains2() {
    char *input1 = strdup("testst&/Djs()ring1");
    char *c = strdup("&/sdDjs()");
    bool result = contains(input1, c);
    CU_ASSERT_FALSE(result);
}

void test_swap_uint16() {
    uint16_t input = 1;
    uint16_t swapped = _swap_uint16(input);
    CU_ASSERT_EQUAL(swapped, 256U);
}

void test_swap_uint32() {
    uint32_t input = 1;
    uint32_t swapped = _swap_uint32(input);
    CU_ASSERT_EQUAL(swapped, 16777216U);
}

void test_swap_uint64() {
    uint64_t input = 1;
    uint64_t swapped = _swap_uint64(input);
    CU_ASSERT_EQUAL(swapped, 72057594037927936U);
}

// ####################### test case setup ####################### //

CU_TestInfo testcases1[] = {
        {"Test [str_split] 1:", test_str_split1},
        {"Test [str_split] 2:", test_str_split2},
        {"Test [str_split] 3:", test_str_split3},
        {"Test [get_filename_ext] 1:", test_get_filename_ext1},
        {"Test [get_filename_ext] 2:", test_get_filename_ext2},
        {"Test [get_empty_char_buffer]:", test_get_empty_char_buffer},
        {"Test [starts_with] 1:", test_starts_with1},
        {"Test [starts_with] 2:", test_starts_with2},
        {"Test [get_string_between_delimiters] 1:", test_get_string_between_delimiters1},
        {"Test [get_string_between_delimiters] 2:", test_get_string_between_delimiters2},
        {"Test [remove_leading_spaces]:", test_remove_leading_spaces},
        {"Test [concat_path_filename]:", test_concat_path_filename},
        {"Test [get_filename_from_path]:", test_get_filename_from_path},
        {"Test [int32_to_str]:", test_int32_to_str},
        {"Test [number_of_digits]:", test_number_of_digits},
        {"Test [add_square_brackets]:", test_add_square_brackets},
        {"Test [add_equals_sign]:", test_add_equals_sign},
        {"Test [contains] 1:", test_contains1},
        {"Test [contains] 2:", test_contains2},
        CU_TEST_INFO_NULL
};

CU_TestInfo testcases2[] = {
        {"Test [swap_uint16]:", test_swap_uint16},
        {"Test [swap_uint32]:", test_swap_uint32},
        {"Test [swap_uint64]:", test_swap_uint64},
        CU_TEST_INFO_NULL
};

CU_SuiteInfo suites1[] = {
        {"Testing utils.c [STRINGS]:", NULL, NULL, NULL, NULL, testcases1},
        {"Testing utils.c [BYTES]:", NULL, NULL, NULL, NULL, testcases2},
        CU_SUITE_INFO_NULL
};

void AddTestsUtils(void)
{
        assert(NULL != CU_get_registry());
        assert(!CU_is_test_running());

        if(CUE_SUCCESS != CU_register_suites(suites1)){
                fprintf(stderr, "Register suites failed - %s ", CU_get_error_msg());
                exit(1);
        }
}