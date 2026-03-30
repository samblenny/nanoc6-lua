#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "utf8.h"

#define LINE_BUFFER_SIZE 256

// State machine for escape sequences
typedef enum {
    STATE_NORMAL,
    STATE_ESC,
    STATE_BRACKET
} input_state_t;

typedef struct Context {
    char *line_buffer;
    size_t line_buffer_size;
    int pos;
    int end_pos;
} context_t;

// Clear from cursor to end of line, then redraw
static void redraw_from_cursor(context_t *ctx)
{
    // Clear from cursor position to end of line
    write(STDOUT_FILENO, "\e[K", 3);

    // Redraw from cursor to end of buffer
    void * start = ctx->line_buffer + ctx->pos;
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
        while (del_pos > 0 && (ctx->line_buffer[del_pos] & 0xC0) == 0x80) {
            del_pos--;
        }
        int char_bytes = ctx->pos - del_pos;

        // Shift remaining characters left in buffer
        for (int i = ctx->pos; i < ctx->end_pos; i++) {
            ctx->line_buffer[i - char_bytes] = ctx->line_buffer[i];
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
    // Validate state before processing
    if (ctx->pos < 0 || ctx->pos > ctx->end_pos ||
        ctx->end_pos > ctx->line_buffer_size - 1)
    {
        return;  // Corrupted state, bail out
    }

    if (ch == '\n') {
        // Line ending: rx line endings already normalized \r to \n
        return;  // Will be handled in read_line
    } else if (ch == 0x08 || ch == 0x7f) {
        // backspace or DEL
        delete_char_at(ctx);
    } else if (ch == 0x03) {
        // TODO: handle Ctrl+C
    } else if (ch == '\t' || ch >= 32) {
        // tab, printable ASCII, or UTF-8 byte
        if (ctx->end_pos < ctx->line_buffer_size - 1) {
            // Shift characters right to make room for insertion
            for (int i = ctx->end_pos; i > ctx->pos; i--) {
                ctx->line_buffer[i] = ctx->line_buffer[i - 1];
            }
            ctx->line_buffer[ctx->pos] = ch;
            write(STDOUT_FILENO, &ch, 1);
            (ctx->pos)++;
            (ctx->end_pos)++;
            if (ctx->pos != ctx->end_pos) {
                redraw_from_cursor(ctx);
            }
        }
        // else silently ignore if buffer full
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
               (ctx->line_buffer[new_pos] & 0xC0) == 0x80)
        {
            new_pos--;
        }

        // Check if we're at a ZWJ (0xE2 0x80 0x8D in UTF-8)
        if (new_pos + 2 < ctx->pos &&
            ctx->line_buffer[new_pos] == 0xE2 &&
            ctx->line_buffer[new_pos + 1] == 0x80 &&
            ctx->line_buffer[new_pos + 2] == 0x8D)
        {
            // This is a ZWJ marker, walk back to the codepoint before it
            new_pos--;
            while (new_pos > 0 &&
                   (ctx->line_buffer[new_pos] & 0xC0) == 0x80)
            {
                new_pos--;
            }
        }

        // Get display width of the character we're moving back from
        int width = utf8_character_width(ctx->line_buffer, new_pos,
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
               (ctx->line_buffer[new_pos] & 0xC0) == 0x80)
        {
            new_pos++;
        }

        // Check if next codepoint is ZWJ (0xE2 0x80 0x8D)
        // If so, skip the ZWJ and any following emoji
        while (new_pos + 2 < ctx->end_pos &&
               ctx->line_buffer[new_pos] == 0xE2 &&
               ctx->line_buffer[new_pos + 1] == 0x80 &&
               ctx->line_buffer[new_pos + 2] == 0x8D)
        {
            // Skip ZWJ (3 bytes)
            new_pos += 3;

            // Skip the next codepoint after ZWJ
            while (new_pos < ctx->end_pos &&
                   (ctx->line_buffer[new_pos] & 0xC0) == 0x80)
            {
                new_pos++;
            }
        }

        // Get display width of the character we're moving to
        int width = utf8_character_width(ctx->line_buffer, ctx->pos,
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
        if (ctx->end_pos == 0 && ctx->line_buffer[0] != 0) {
            ctx->end_pos = strlen(ctx->line_buffer);
            ctx->pos = ctx->end_pos;
            write(STDOUT_FILENO, ctx->line_buffer, ctx->end_pos);
        }
    } else if (ch == 'B') {
        // Ignore down arrow (B)
    }
    // Ignore other escape sequences
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
    context_t ctx = {
        .line_buffer = line_buffer,
        .line_buffer_size = bufsize,
        .pos = 0,      // cursor position
        .end_pos = 0,  // end of input
    };

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
                handle_normal_char(ch, &ctx);
                if (ch == '\n') {
                    ctx.line_buffer[ctx.end_pos] = 0;
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
