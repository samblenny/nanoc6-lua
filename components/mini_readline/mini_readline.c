#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utf8.h"

// State machine for escape sequences
typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_BRACKET
} input_state_t;

typedef struct Context {
    char *buf;
    size_t buf_size;
    int pos;
    int end_pos;
} context_t;

// Clear from cursor to end of line, then redraw
static void redraw_from_cursor(context_t *ctx)
{
    // Clear
    write(STDOUT_FILENO, "\e[K", 3);

    // Redraw
    void * start = ctx->buf + ctx->pos;
    int num_bytes = ctx->end_pos - ctx->pos;
    write(STDOUT_FILENO, start, num_bytes);

    // Move cursor back to current position by sending backspaces
    // for each byte we just wrote (terminal handles UTF-8 rendering)
    for (int i = 0; i < num_bytes; i++) {
        write(STDOUT_FILENO, "\b", 1);
    }
}

// Delete character at current position (for backspace)
static void delete_char_at(context_t *ctx)
{
    if (ctx->pos > 0 && ctx->pos <= ctx->end_pos && ctx->end_pos > 0) {
        // Walk backwards to find UTF-8 character start
        int del_pos = ctx->pos - 1;
        while (del_pos > 0 && (ctx->buf[del_pos] & 0xC0) == 0x80) {
            del_pos--;
        }
        int char_bytes = ctx->pos - del_pos;

        // Shift remaining characters left in buffer
        for (int i = ctx->pos; i < ctx->end_pos; i++) {
            ctx->buf[i - char_bytes] = ctx->buf[i];
        }

        // Update positions
        ctx->pos = del_pos;
        ctx->end_pos -= char_bytes;

        // Move cursor back and redraw from new position
        for (int i = 0; i < char_bytes; i++) {
            write(STDOUT_FILENO, "\b", 1);
        }
        redraw_from_cursor(ctx);
    }
}

// Handle a single character in normal mode
static void handle_normal_char(unsigned char ch, context_t *ctx)
{
    if (ctx->pos < 0) {
        printf("\r\nERR ctx->pos < 0\n\n");
        return;
    } else if (ctx->pos > ctx->end_pos) {
        printf("\r\nERR ctx->pos > ctx->end_pos\n\n");
        return;
    } else if (ctx->pos > ctx->buf_size - 1) {
        printf("\r\nERR ctx->pos > ctx->buf_size -1\n\n");
        return;
    }

    switch(ch) {
    case 0x03: // Ctrl-C
    case '\t': // TAB
    case '\n': // LF: note that rx line endings normalizes CR to LF
        break;
    case 0x08: // BS
    case 0x7f: // DEL
        delete_char_at(ctx);
        break;
    default:
        if (ch < 32) {
            // Silently ignore other control characters
            break;
        }
        // At this point, ch should be printable ASCII or a UTF-8 byte
        if (ctx->end_pos >= ctx->buf_size -1) {
            // silently truncate input when buffer is full
            return;
        }
        // check if this is a mid-buffer insert or an end-buffer append
        bool insert = ctx->pos < ctx->end_pos;

        // Insert or append the new character
        (ctx->end_pos)++;
        if (insert) {
            // Mid-buffer insert: shift characters right to make room
            for (int i = ctx->end_pos; i > ctx->pos; i--) {
                ctx->buf[i] = ctx->buf[i - 1];
            }
        }
        // Set the new character and update cursor position
        ctx->buf[ctx->pos] = ch;
        (ctx->pos)++;

        // Update the terminal
        write(STDOUT_FILENO, &ch, 1);
        if (insert) {
            redraw_from_cursor(ctx);
        }
    }
}

// Move cursor left in buffer and on screen
// UTF-8 aware: respects character boundaries and emoji/ZWJ sequences
static void move_cursor_left(context_t *ctx)
{
    if (ctx->pos > 0) {
        int new_pos = ctx->pos - 1;

        // Walk back to UTF-8 character start (find non-continuation byte)
        while (new_pos > 0 &&
               (ctx->buf[new_pos] & 0xC0) == 0x80)
        {
            new_pos--;
        }

        // Check if we're at a ZWJ (0xE2 0x80 0x8D in UTF-8)
        if (new_pos + 2 < ctx->pos &&
            ctx->buf[new_pos] == 0xE2 &&
            ctx->buf[new_pos + 1] == 0x80 &&
            ctx->buf[new_pos + 2] == 0x8D)
        {
            // This is a ZWJ marker, walk back to the codepoint before it
            new_pos--;
            while (new_pos > 0 &&
                   (ctx->buf[new_pos] & 0xC0) == 0x80)
            {
                new_pos--;
            }
        }

        // Get display width of the character we're moving back from
        int width = utf8_character_width(ctx->buf, new_pos,
            ctx->end_pos);

        // Send backspaces equal to display width
        for (int i = 0; i < width; i++) {
            write(STDOUT_FILENO, "\x1b[D", 3);  // ESC [ D
        }

        ctx->pos = new_pos;
    }
}

// Move cursor right in buffer and on screen
// UTF-8 aware: respects character boundaries and emoji/ZWJ sequences
static void move_cursor_right(context_t *ctx)
{
    if (ctx->pos < ctx->end_pos) {
        int new_pos = ctx->pos;

        // Move to next byte
        new_pos++;

        // Skip any continuation bytes (UTF-8 bytes matching 10xxxxxx)
        while (new_pos < ctx->end_pos &&
               (ctx->buf[new_pos] & 0xC0) == 0x80)
        {
            new_pos++;
        }

        // Check if next codepoint is ZWJ (0xE2 0x80 0x8D)
        // If so, skip the ZWJ and any following emoji
        while (new_pos + 2 < ctx->end_pos &&
               ctx->buf[new_pos] == 0xE2 &&
               ctx->buf[new_pos + 1] == 0x80 &&
               ctx->buf[new_pos + 2] == 0x8D)
        {
            // Skip ZWJ (3 bytes)
            new_pos += 3;

            // Skip the next codepoint after ZWJ
            while (new_pos < ctx->end_pos &&
                   (ctx->buf[new_pos] & 0xC0) == 0x80)
            {
                new_pos++;
            }
        }

        // Get display width of the character we're moving to
        int width = utf8_character_width(ctx->buf, ctx->pos,
            ctx->end_pos);

        // Send forward escapes equal to display width
        for (int i = 0; i < width; i++) {
            write(STDOUT_FILENO, "\x1b[C", 3);  // ESC [ C
        }

        ctx->pos = new_pos;
    }
}

// Handle escape sequence continuation (arrow keys)
static void handle_escape_char(unsigned char ch, context_t *ctx)
{
    if (ch == 'C') {
        // Right arrow
        move_cursor_right(ctx);
    } else if (ch == 'D') {
        // Left arrow
        move_cursor_left(ctx);
    } else if (ch == 'A') {
        // Up arrow: if current line is empty, load previous input line
        if (ctx->end_pos == 0 && ctx->buf[0] != 0) {
            ctx->end_pos = strlen(ctx->buf);
            ctx->pos = ctx->end_pos;
            write(STDOUT_FILENO, ctx->buf, ctx->end_pos);
        }
    } else if (ch == 'B') {
        // Ignore down arrow (B)
    }
    // Ignore other escape sequences
}

// Reads a line of input into buffer, handles UTF-8 and cursor movement.
// Returns: length (excluding null terminator), 0 for empty, -1 on error
int mini_readline(const char *prompt, char *buf, int bufsize)
{
    if (prompt == NULL || buf == NULL || bufsize <= 1) {
        return -1;
    }

    input_state_t state = STATE_NORMAL;
    context_t ctx = {
        .buf = buf,
        .buf_size = bufsize,
        .pos = 0,      // cursor position
        .end_pos = 0,  // end of input
    };

    write(STDOUT_FILENO, prompt, strlen(prompt));

    while (1) {
        unsigned char ch;
        if (read(STDIN_FILENO, &ch, 1) <= 0) {
            // read error or no data available
            vTaskDelay(pdMS_TO_TICKS(10));
            continue;
        }

        // Dispatch based on current state
        if (state == STATE_NORMAL) {
            if (ch == 0x1B) {
                // Escape sequence start
                state = STATE_ESC;
            } else {
                handle_normal_char(ch, &ctx);
                if (ch == '\n') {
                    ctx.buf[ctx.end_pos] = 0;
                    write(STDOUT_FILENO, "\r\n", 2);
                    return ctx.end_pos;
                }
            }
        } else if (state == STATE_ESC) {
            if (ch == '[') {
                state = STATE_BRACKET;
            } else {
                state = STATE_NORMAL;  // Invalid sequence, reset
            }
        } else if (state == STATE_BRACKET) {
            handle_escape_char(ch, &ctx);
            state = STATE_NORMAL;  // Escape sequence complete
        }
    }
}
