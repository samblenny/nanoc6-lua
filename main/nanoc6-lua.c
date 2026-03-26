#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/usb_serial_jtag.h"
#include "driver/usb_serial_jtag_vfs.h"
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
#include <unistd.h>  /* for fsync */
#include <fcntl.h>


static int usb_readline(char *buf, int maxlen) {
    int i = 0;
    while (i < maxlen - 1) {
        int c = fgetc(stdin);
        if (c == EOF) {
            vTaskDelay(pdMS_TO_TICKS(10)); // spurious EOF, yield and retry
            continue;
        }
        fputc(c, stdout); // echo
        fflush(stdout);
        fsync(fileno(stdout));  // force USB packet
        if (c == '\n' || c == '\r') {
            break;
        }
        buf[i++] = (char)c;
    }
    buf[i] = '\0';
    return i;
}

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

void app_main(void) {
    // Try to make USB serial input work better
    // see esp-idf/examples/system/console/advanced/main/console_settings.c
    usb_serial_jtag_vfs_set_rx_line_endings(ESP_LINE_ENDINGS_CR);
    usb_serial_jtag_vfs_set_tx_line_endings(ESP_LINE_ENDINGS_CRLF);

    // Blocking mode
    fcntl(fileno(stdout), F_SETFL, 0);
    fcntl(fileno(stdin), F_SETFL, 0);

    // Begin using interrupt-driven driver
    usb_serial_jtag_driver_config_t cfg = {
        .rx_buffer_size = 256,
        .tx_buffer_size = 256,
    };
    usb_serial_jtag_driver_install(&cfg);
    usb_serial_jtag_vfs_use_driver();

    // Disable stream buffering
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    // start interpreter with 64KB stack
    xTaskCreate(lua_task, "lua", 65536, NULL, 5, NULL);
    // make sure app_main exiting doesn't cause problems
    vTaskDelete(NULL);
}
