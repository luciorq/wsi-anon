#include "CUnit/Basic.h"

#include "../../src/wsi-anonymizer.h"

// ####################### functions to test ####################### //

extern int32_t anonymize_wsi_inplace(const char *filename, const char *new_label_name, bool keep_macro_image,
                                     bool disable_unlinking);

// ####################### test cases ####################### //

void test_errors_are_propagated() {
    int32_t result = anonymize_wsi_inplace("/non/existing/wsi.svs", "new_label", false, false);
    CU_ASSERT_NOT_EQUAL(result, 0);
}

// ####################### test case setup ####################### //

CU_TestInfo anonymize_wsi_tests[] = {{"Test [anonymize_wsi_inplace] 1:", test_errors_are_propagated},
                                     CU_TEST_INFO_NULL};

CU_SuiteInfo anonymize_wsi_test_suite[] = {{"Testing wsi-anonymizer.c:", NULL, NULL, NULL, NULL, anonymize_wsi_tests},
                                           CU_SUITE_INFO_NULL};

void AddTestsWsiAnonymizer(void) {
    assert(NULL != CU_get_registry());
    assert(!CU_is_test_running());

    if (CUE_SUCCESS != CU_register_suites(anonymize_wsi_test_suite)) {
        fprintf(stderr, "Register suites failed - %s ", CU_get_error_msg());
        exit(1);
    }
}
