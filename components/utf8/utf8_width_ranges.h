// Character width ranges from ../../util/wcwidth_distiller/distill
// Covers: ASCII, Latin, Greek, Cyrillic, CJK, Hiragana, Katakana, Hangul,
// punctuation, and math.
// Emoji ranges are hardcoded as width 2 on the assumption they will appear
// with the FE0F variation selector in cases when an older width 1 glyph is
// also possible.

#ifndef UTF8_WIDTH_RANGES_H
#define UTF8_WIDTH_RANGES_H

#include <stdint.h>

typedef struct {
    uint32_t start;
    uint32_t end;
    int width;
} Utf8WidthRange;

// This list is approximate and incomplete, but good enough for my purposes.
// CRITICAL: If you modify this list, ensure it stays sorted!
static const Utf8WidthRange utf8_width_ranges[] = {
    {0x000000, 0x000000, 0},  // NULL
    {0x000020, 0x00007E, 1},  // ASCII printable
    {0x0000A0, 0x0000AC, 1},  // Latin-1 Supplement (no-break space, etc)
    {0x0000AE, 0x00024F, 1},  // Latin Extended-A/B
    {0x000300, 0x00036F, 0},  // Combining Diacritical Marks
    {0x000370, 0x000482, 1},  // Greek + Cyrillic
    {0x000483, 0x000489, 0},  // Cyrillic combining marks
    {0x00048A, 0x00052F, 1},  // Cyrillic
    {0x001100, 0x00115F, 2},  // Hangul Jamo
    {0x001160, 0x0011FF, 0},  // Hangul Jamo (combining)
    {0x002000, 0x00200A, 1},  // General Punctuation (spaces)
    {0x00200B, 0x00200F, 0},  // Zero-width characters + directionality marks
    {0x002010, 0x002027, 1},  // General Punctuation (dashes, quotes)
    {0x00202F, 0x00205F, 1},  // General Punctuation (narrow no-break space, etc)
    {0x002060, 0x002064, 0},  // Word joiner, zero-width no-break space, invisible separator
    {0x002070, 0x002079, 1},  // Superscripts
    {0x002200, 0x0022FF, 1},  // Mathematical Operators
    {0x003041, 0x003096, 2},  // Hiragana
    {0x003099, 0x00309A, 0},  // Hiragana combining marks
    {0x00309B, 0x0030FF, 2},  // Hiragana + Katakana
    {0x003400, 0x004DBF, 2},  // CJK Unified Ideographs Extension A
    {0x004E00, 0x009FFF, 2},  // CJK Unified Ideographs
    {0x00AC00, 0x00D7A3, 2},  // Hangul Syllables
    {0x00FE00, 0x00FE0F, 0},  // Variation Selectors (emoji presentation)

    // Emoji blocks (hardcoded as width 2, macOS emoji picker adds FE0F)
    {0x01F300, 0x01F64F, 2},  // various emoji blocks
    {0x01F680, 0x01FAFF, 2},  // various emoji blocks
};

static const int utf8_width_ranges_count = sizeof(utf8_width_ranges) /
    sizeof(utf8_width_ranges[0]);

#endif // UTF8_WIDTH_RANGES_H
