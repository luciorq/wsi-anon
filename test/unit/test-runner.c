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

        unsigned int num_fails = CU_get_number_of_failures();
        result = (num_fails != 0);

        CU_cleanup_registry();
    }
    return result;
}
