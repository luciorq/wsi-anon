#include "CUnit/Basic.h"
#include <CUnit/Automated.h>
#include <stdio.h>
#include <stdlib.h>

#include "ini-parser-test.h"
#include "utils-test.h"
#include "wsi-anonymizer-test.h"

int main() {
    int result = 1;
    if (CU_initialize_registry()) {
        fprintf(stderr, "Initialization of Test Registry failed.\n");
        exit(result);
    } else {
        fprintf(stdout, "Initializing tests\n");
        AddTestsUtils();
        AddTestsIniParser();
        AddTestsWsiAnonymizer();
        
        fprintf(stdout, "Set output filename to Test-Wsi-Anon\n");
        CU_set_output_filename("Test-Wsi-Anon");
        
        fprintf(stdout, "Running tests...\n");
        CU_automated_run_tests();

        fprintf(stdout, "_______________________________________________\n");
        unsigned int num_run = CU_get_number_of_tests_run();
        unsigned int num_inactive = CU_get_number_of_tests_inactive();
        unsigned int num_fails = CU_get_number_of_tests_failed();
        fprintf(stdout, "Result: %u succeeded - %u skipped - %u failed\n", num_run, num_inactive,
                num_fails);

        CU_pFailureRecord records = CU_get_failure_list();
        while (records++ != NULL) {
            fprintf(stdout, "\n* Error in test %s\n", records->pTest->pName);
        }
        
        result = (num_fails != 0);
        CU_cleanup_registry();
    }
    return result;
}
