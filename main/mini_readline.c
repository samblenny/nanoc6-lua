#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define LINE_BUFFER_SIZE 256

// State machine for escape sequences
typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_BRACKET
} input_state_t;

// Clear from cursor to end of line, then redraw
static void redraw_from_cursor(char *line_buffer, int *pos, int *end_pos)
{
    // Clear from cursor postion to end of line
    write(STDOUT_FILENO, "\e[K", 3);

    // Redraw from cursor to end of buffer
    write(STDOUT_FILENO, line_buffer + *pos, *end_pos - *pos);

    // Put cursor back where it belongs
    for (int i = *end_pos - *pos; i > 0; i--) {
        write(STDOUT_FILENO, "\b", 1);
    }
}

// Delete character at current position (for backspace)
static void delete_char_at(char *line_buffer, int *pos, int *end_pos)
{
    if (*pos > 0 && *pos <= *end_pos && *end_pos > 0) {
        // Walk backwards to find UTF-8 character start
        int del_pos = *pos - 1;
        while (del_pos > 0 && (line_buffer[del_pos] & 0xC0) == 0x80) {
            del_pos--;
        }
        int char_bytes = *pos - del_pos;

        // Erase char_bytes from screen
        for (int i = 0; i < char_bytes; i++) {
            write(STDOUT_FILENO, "\b \b", 3);
        }

        // Shift remaining characters left in buffer
        for (int i = *pos; i < *end_pos; i++) {
            line_buffer[i - char_bytes] = line_buffer[i];
        }

        // Update positions
        *pos = del_pos;
        *end_pos -= char_bytes;
        redraw_from_cursor(line_buffer, pos, end_pos);
    }
}

// Handle a single character in normal mode
static void handle_normal_char(unsigned char ch, char *line_buffer,
    int bufsize, int *pos, int *end_pos)
{
    // Validate state before processing
    if (*pos < 0 || *pos > *end_pos || *end_pos > bufsize - 1) {
        return;  // Corrupted state, bail out
    }

    if (ch == '\n') {
        // Line ending: rx line endings already normalized \r to \n
        return;  // Will be handled in read_line
    } else if (ch == 0x08 || ch == 0x7f) {
        // backspace or DEL
        delete_char_at(line_buffer, pos, end_pos);
    } else if (ch == 0x03) {
        // TODO: handle Ctrl+C
    } else if (ch == '\t' || ch >= 32) {
        // tab, printable ASCII, or UTF-8 byte
        if (*end_pos < bufsize - 1) {
            // Shift characters right to make room for insertion
            for (int i = *end_pos; i > *pos; i--) {
                line_buffer[i] = line_buffer[i - 1];
            }
            line_buffer[*pos] = ch;
            write(STDOUT_FILENO, &ch, 1);
            (*pos)++;
            (*end_pos)++;
            if (*pos != *end_pos) {
                redraw_from_cursor(line_buffer, pos, end_pos);
            }
        }
        // else silently ignore if buffer full
    }
}

// Move cursor left in buffer and on screen
static void move_cursor_left(int *pos)
{
    if (*pos > 0) {
        (*pos)--;
        write(STDOUT_FILENO, "\x1b[D", 3);  // ESC [ D
    }
}

// Move cursor right in buffer and on screen
static void move_cursor_right(int *pos, int end_pos)
{
    if (*pos < end_pos) {
        (*pos)++;
        write(STDOUT_FILENO, "\x1b[C", 3);  // ESC [ C
    }
}

// Handle escape sequence continuation (arrow keys)
static void handle_escape_char(unsigned char ch, int *pos, int end_pos)
{
    if (ch == 'C') {
        // Right arrow
        move_cursor_right(pos, end_pos);
    } else if (ch == 'D') {
        // Left arrow
        move_cursor_left(pos);
    }
    // Ignore up/down arrows (A and B)
}

// Reads a line of input into buffer, handles UTF-8 and cursor movement.
// Returns: length (excluding null terminator), 0 for empty, -1 on error
int mini_readline(const char *prompt, char *line_buffer, int bufsize)
{
    // Validate inputs
    if (prompt == NULL || line_buffer == NULL || bufsize <= 1) {
        return -1;
    }

    input_state_t state = STATE_NORMAL;
    int pos = 0;      // cursor position
    int end_pos = 0;  // end of input

    write(STDOUT_FILENO, prompt, strlen(prompt));

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

        // Dispatch based on current state
        if (state == STATE_NORMAL) {
            if (ch == 0x1B) {
                // Escape sequence start
                state = STATE_ESC;
            } else {
                handle_normal_char(ch, line_buffer, bufsize, &pos, &end_pos);
                if (ch == '\n') {
                    line_buffer[end_pos] = 0;
                    write(STDOUT_FILENO, "\r\n", 2);
                    return end_pos;
                }
            }
        } else if (state == STATE_ESC) {
            if (ch == '[') {
                state = STATE_BRACKET;
            } else {
                state = STATE_NORMAL;  // Invalid sequence, reset
            }
        } else if (state == STATE_BRACKET) {
            handle_escape_char(ch, &pos, end_pos);
            state = STATE_NORMAL;  // Escape sequence complete
        }
    }
}
