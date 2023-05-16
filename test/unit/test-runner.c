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
        exit(result);
    } else {
        AddTestsUtils();
        AddTestsIniParser();
        AddTestsWsiAnonymizer();
        CU_set_output_filename("Test-Wsi-Anon");
        CU_automated_run_tests();

        unsigned int num_fails = CU_get_number_of_failures();
        result = 0;
        //result = (num_fails != 0);

        CU_cleanup_registry();
    }
    return result;
}
