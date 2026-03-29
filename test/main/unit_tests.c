#include <stdio.h>
#include <string.h>
#include "unity.h"

const char *banner =
"\n"
"==========================================================================\n"
"=== Running Unit Tests ===================================================\n"
"==========================================================================\n"
"\n";

void app_main(void)
{
    printf(banner);
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
}
