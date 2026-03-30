#include <stdint.h>
#include "utf8_width_ranges.h"

// Get display width of a codepoint using embedded lookup table
// Returns: 0 for zero-width (combining, ZWJ, variation selectors)
//          1 for narrow (ASCII, Latin, Cyrillic, etc)
//          2 for wide (CJK, Hiragana, Katakana, Hangul)
//         -1 for control/unprintable (not in lookup table)
int utf8_codepoint_width(uint32_t cp)
{
    // Linear search through width ranges
    for (int i = 0; i < utf8_width_ranges_count; i++) {
        if (cp >= utf8_width_ranges[i].start &&
            cp <= utf8_width_ranges[i].end)
        {
            return utf8_width_ranges[i].width;
        } else if (cp < utf8_width_ranges[i].start) {
            // ranges are sorted; this range is above codepoint, so bail out
            return -1;
        }
    }
    return -1;  // Default: control/unprintable/unassigned
}

// Decode UTF-8 codepoint at position in buffer
// Returns the codepoint value
uint32_t utf8_decode_codepoint(const char *buf, int pos, int end)
{
    if (pos >= end) {
        return 0;
    }

    unsigned char first = buf[pos];

    // Single-byte ASCII (0xxxxxxx)
    if ((first & 0x80) == 0) {
        return (uint32_t)first;
    }

    // Two-byte sequence (110xxxxx 10xxxxxx)
    if ((first & 0xE0) == 0xC0 && pos + 1 < end) {
        return ((uint32_t)(first & 0x1F) << 6) |
               ((uint32_t)(buf[pos + 1] & 0x3F));
    }

    // Three-byte sequence (1110xxxx 10xxxxxx 10xxxxxx)
    if ((first & 0xF0) == 0xE0 && pos + 2 < end) {
        return ((uint32_t)(first & 0x0F) << 12) |
               ((uint32_t)(buf[pos + 1] & 0x3F) << 6) |
               ((uint32_t)(buf[pos + 2] & 0x3F));
    }

    // Four-byte sequence (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
    if ((first & 0xF8) == 0xF0 && pos + 3 < end) {
        return ((uint32_t)(first & 0x07) << 18) |
               ((uint32_t)(buf[pos + 1] & 0x3F) << 12) |
               ((uint32_t)(buf[pos + 2] & 0x3F) << 6) |
               ((uint32_t)(buf[pos + 3] & 0x3F));
    }

    // Invalid UTF-8
    return 0xFFFD;
}

// Get display width of a UTF-8 codepoint at buffer position
// Decodes the character at (buf, pos) and returns its display width
int utf8_character_width(const char *buf, int pos, int end)
{
    uint32_t cp = utf8_decode_codepoint(buf, pos, end);
    return utf8_codepoint_width(cp);
}
