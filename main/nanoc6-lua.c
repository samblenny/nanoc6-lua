#include <stdio.h>
#include <errno.h>
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

#define LINE_BUFFER_SIZE 256

// Reads a line of input into buffer, handles UTF-8 and backspace.
// Returns: length (excluding null terminator), 0 for empty, -1 on error
static int read_line(const char *prompt, char *line_buffer, int bufsize) {
    write(STDOUT_FILENO, prompt, strlen(prompt));

    int pos = 0;
    while (1) {
        unsigned char ch;
        ssize_t n = read(STDIN_FILENO, &ch, 1);

        if (n < 0) {
            // read error
            if (errno == EINTR) {
                continue;  // retry on signal
            }
            return -1;
        }
        if (n == 0) {
            // no data available
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        if (ch == '\n') {
            // Line ending: rx line endings already normalized \r to \n
            line_buffer[pos] = 0;
            write(STDOUT_FILENO, "\r\n", 2);
            return pos;
        } else if (ch == 0x08 || ch == 0x7f) {
            // backspace or DEL
            if (pos > 0) {
                // Walk backwards to find UTF-8 character start.
                // UTF-8 start bytes: 0xxxxxxx or 11xxxxxx
                // Continuation bytes: 10xxxxxx
                int del_pos = pos - 1;
                while (del_pos > 0 && (line_buffer[del_pos] & 0xC0) == 0x80) {
                    del_pos--;
                }
                int char_bytes = pos - del_pos;
                pos = del_pos;

                // Erase char_bytes from screen
                for (int i = 0; i < char_bytes; i++) {
                    write(STDOUT_FILENO, "\b \b", 3);
                }
            }
        } else if (ch == 0x03) {
            // TODO: handle Ctrl+C
        } else if (ch == '\t' || ch >= 32) {
            // tab, printable ASCII, or UTF-8 byte
            if (pos < bufsize - 1) {
                line_buffer[pos++] = ch;
                write(STDOUT_FILENO, &ch, 1);
            }
            // else silently ignore if buffer full, but loop continues
        }
    }
}


// Lua gets its own task so it can have a bigger stack than app_main default
static void lua_task(void *pvParameters) {
    char line_buffer[LINE_BUFFER_SIZE];
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    while(1) {
        // Read a line (chunk) of input
        int len = read_line("> ", line_buffer, LINE_BUFFER_SIZE);
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
