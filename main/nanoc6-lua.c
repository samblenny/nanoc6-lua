#include <stdio.h>
#include "esp_system.h"  /* ESP_ERR_CHECK */
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
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

// Prepare the serial driver and linenoise for REPL line editing input
void initializeUSBSerial() {
    // Treat CR or CRLF as newline
    usb_serial_jtag_vfs_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    usb_serial_jtag_vfs_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    // Use blocking mode IO
    fcntl(fileno(stdout), F_SETFL, 0);
    fcntl(fileno(stdin), F_SETFL, 0);

    usb_serial_jtag_driver_config_t usb_serial_jtag_config =
        USB_SERIAL_JTAG_DRIVER_CONFIG_DEFAULT();

    // Begin using USB serial JTAG driver
    esp_err_t ok;
    ok = usb_serial_jtag_driver_install(&usb_serial_jtag_config);
    if(ok != ESP_OK) {
        // TODO: handle this better
        printf("This is bad\n");
    }
    usb_serial_jtag_vfs_use_driver();

    // This makes linenoise act as a vanilla dumb terminal readline without
    // any of its default weirdness. Without this, the cursor flickers between
    // the edit point and the first character of the line.
    linenoiseSetDumbMode(1);
}

void app_main(void) {
    initializeUSBSerial();

    // A simple echo loop
    while(1) {
        char *line = linenoise("> ");
        if(line != NULL && line[0] != 0) {
            printf("%s\n", line);
        }
        linenoiseFree(line);
    }

    // start interpreter with 64KB stack
//    xTaskCreate(lua_task, "lua", 65536, NULL, 5, NULL);
    // make sure app_main exiting doesn't cause problems
//    vTaskDelete(NULL);
}
