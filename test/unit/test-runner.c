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
        printf(stderr, "Initialization of Test Registry failed.\n");
        exit(result);
    } else {
        printf(stdout, "Initializing tests\n");
        AddTestsUtils();
        AddTestsIniParser();
        AddTestsWsiAnonymizer();

        printf(stdout, "Set output filename to Test-Wsi-Anon\n");
        CU_set_output_filename("Test-Wsi-Anon");

        printf(stdout, "Running tests...\n");
        CU_automated_run_tests();

        printf(stdout, "_______________________________________________\n");
        CU_pRunSummary summary = CU_get_run_summary();
        printf(stdout, "Result: %u succeeded - %u skipped - %u failed\n", summary->nTestsRun,
                summary->nTestsInactive, summary->nTestsFailed);

        result = (summary->nTestsFailed != 0);
        CU_cleanup_registry();
    }
    return result;
}
