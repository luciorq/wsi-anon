#include <stdio.h>
#include <stdlib.h>
#include "CUnit/Basic.h"
#include <CUnit/Automated.h>

#include "ini-parser-test.h"
#include "utils-test.h"

int main() {
   if(CU_initialize_registry()){
        fprintf(stderr, " Initialization of Test Registry failed. ");
        exit(1);
    }else{
        AddTestsUtils();
        AddTestsIniParser();
        CU_set_output_filename("Test-Wsi-Anon");
        //CU_list_tests_to_file();
        CU_automated_run_tests();
        CU_cleanup_registry();
    }
    return 0;
}