#include <stdio.h>
#include "esp_system.h"  /* ESP_ERR_CHECK */
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <unistd.h>  /* fsync */
#include <fcntl.h>
#include "mini_readline.h"

#define LINE_BUFFER_SIZE (128)

// Lua gets its own task so it can have a bigger stack than app_main default
static void lua_task(void *pvParameters) {
    char line_buffer[LINE_BUFFER_SIZE];
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    while(1) {
        // Read a line (chunk) of input
        int len = mini_readline("> ", line_buffer, LINE_BUFFER_SIZE);
        if (len < 0) {
            // read error
            continue;
        } else if (len == 0) {
            // empty line
            continue;
        }

        // Feed the chunk to Lua interperter
        if (luaL_loadstring(L, line_buffer) || lua_pcall(L, 0, 0, 0)) {
            printf("%s\n", lua_tostring(L, -1));
            lua_pop(L, 1);
        }
    }

    lua_close(L);
    vTaskDelete(NULL);
}


// Prepare the serial driver for raw I/O
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
}

void app_main(void) {
    initializeUSBSerial();

    // start interpreter with 64KB stack
    xTaskCreate(lua_task, "lua", 65536, NULL, 5, NULL);
    // make sure app_main exiting doesn't cause problems
    vTaskDelete(NULL);
}
