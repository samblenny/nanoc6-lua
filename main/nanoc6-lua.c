#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void app_main(void)
{
    for(;;) {
        printf("TODO: Lua interpreter here\n");
        vTaskDelay(pdMS_TO_TICKS(5000)); // sleep 5 seconds
    }
}
