#include <stdio.h>
#include "esp_system.h"  /* ESP_ERR_CHECK */
#include "esp_console.h"
#include "linenoise/linenoise.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <unistd.h>  /* fsync */
#include <fcntl.h>

/*
// Lua gets its own task so it can have a bigger stack than app_main default
static void lua_task(void *pvParameters) {
    char buff[256];
    int error;
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    while (1) {
        printf("> ");
        fflush(stdout);
        fsync(fileno(stdout));  // force USB packet
        int len = usb_readline(buff, sizeof(buff));
        if (len == 0) {
            continue;
        }
        error = luaL_loadstring(L, buff) || lua_pcall(L, 0, 0, 0);
        if (error) {
            fprintf(stderr, "%s\n", lua_tostring(L, -1));
            fflush(stderr);
            fsync(fileno(stderr));  // force USB packet
            lua_pop(L, 1);
        }
    }

    lua_close(L);
    vTaskDelete(NULL);
}
*/

void app_main(void) {
    esp_console_repl_t *repl = NULL;
    esp_console_repl_config_t repl_config = ESP_CONSOLE_REPL_CONFIG_DEFAULT();
    repl_config.prompt = ">";
    repl_config.max_cmdline_length = 128;

    esp_console_dev_usb_serial_jtag_config_t hw_config = 
        ESP_CONSOLE_DEV_USB_SERIAL_JTAG_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(
        esp_console_new_repl_usb_serial_jtag(&hw_config, &repl_config, &repl)
    );

    // This makes linenoise act as a vanilla dumb terminal readline without
    // any of its default weirdness. Without this, the cursor flickers between
    // the edit point and the first character of the line.
    linenoiseSetDumbMode(1);

    // This is a REPL helper function used by many esp-idf examples
    ESP_ERROR_CHECK(esp_console_start_repl(repl));


    // start interpreter with 64KB stack
//    xTaskCreate(lua_task, "lua", 65536, NULL, 5, NULL);
    // make sure app_main exiting doesn't cause problems
//    vTaskDelete(NULL);
}
