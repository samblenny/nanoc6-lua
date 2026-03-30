#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>
#include <locale.h>
#include <stdint.h>

// Scan up to U+02FA1E (last useful codepoint for practical text rendering)
// Beyond this are mostly rare scripts, private use, and unassigned ranges
// that won't appear in typical text (vocab learning, CJK, emoji)
#define MAX_CODEPOINT 0x02FA1E

typedef struct {
    uint32_t start;
    uint32_t end;
    int width;
    // State tracking for current range being built
    uint32_t current_start;
    int current_width;
    int count;
} WidthRange;

#define MAX_RANGES 100000

void flush_range(WidthRange *wr, uint32_t end) {
    if (wr->current_width != -2) {
        wr->start = wr->current_start;
        wr->end = end;
        wr->width = wr->current_width;
        wr->count++;
    }
}

int main() {
    setlocale(LC_ALL, "en_US.UTF-8");

    // Define target ranges for practical text rendering
    struct {
        uint32_t start;
        uint32_t end;
        const char *name;
    } target_ranges[] = {
        {0x0000, 0x007F, "ASCII"},
        {0x0080, 0x00FF, "Latin-1 Supplement"},
        {0x0100, 0x017F, "Latin Extended-A"},
        {0x0180, 0x024F, "Latin Extended-B"},
        {0x2070, 0x2079, "Superscripts"},
        {0x2200, 0x22FF, "Mathematical Operators"},
        {0x0300, 0x036F, "Combining Diacritical Marks"},
        {0x0370, 0x03FF, "Greek"},
        {0x0400, 0x04FF, "Cyrillic"},
        {0x0500, 0x052F, "Cyrillic Supplement"},
        {0x3040, 0x309F, "Hiragana"},
        {0x30A0, 0x30FF, "Katakana"},
        {0x3400, 0x4DBF, "CJK Unified Ideographs Extension A"},
        {0x4E00, 0x9FFF, "CJK Unified Ideographs"},
        {0xAC00, 0xD7A3, "Hangul Syllables"},
        {0x1100, 0x11FF, "Hangul Jamo"},
        {0x2000, 0x206F, "General Punctuation"},
        {0xFE00, 0xFE0F, "Variation Selectors"},
// These are misleading due to not considering the FEOF variaton selector
//        {0x1F600, 0x1F64F, "Emoticons"},
//        {0x1F300, 0x1F5FF, "Miscellaneous Symbols and Pictographs"},
        {0, 0, NULL}  // sentinel
    };

    static WidthRange ranges[MAX_RANGES];
    WidthRange state = {};
    state.current_start = 0;
    state.current_width = -2;
    state.count = 0;

    printf("Scanning wcwidth() for target ranges...\n\n");

    // Scan each target range
    for (int r = 0; target_ranges[r].name != NULL; r++) {
        uint32_t range_start = target_ranges[r].start;
        uint32_t range_end = target_ranges[r].end;
        const char *range_name = target_ranges[r].name;

        printf("Scanning %s (U+%04X–U+%04X)...\n", range_name, range_start, range_end);

        for (uint32_t cp = range_start; cp <= range_end; cp++) {
            int w = wcwidth((wchar_t)cp);

            // wcwidth returns -1 for control chars, unprintable, etc.
            if (w != state.current_width) {
                flush_range(&state, cp - 1);
                if (state.current_width != -2) {
                    ranges[state.count - 1] = state;
                }
                state.current_start = cp;
                state.current_width = w;
            }
        }

        // Flush range at end of each target block
        flush_range(&state, range_end);
        if (state.current_width != -2) {
            ranges[state.count - 1] = state;
        }
        state.current_width = -2;  // Reset for next range block
    }

    int range_count = state.count;

    printf("Done. Found %d ranges.\n\n", range_count);

    // Filter out -1 width ranges (control/unprintable)
    int filtered_count = 0;
    for (int i = 0; i < range_count; i++) {
        if (ranges[i].width >= 0) {
            ranges[filtered_count] = ranges[i];
            filtered_count++;
        }
    }
    range_count = filtered_count;

    printf("After filtering out -1 results: %d ranges.\n\n", range_count);

    // Merge adjacent ranges with same width
    int merged_count = 0;
    for (int i = 0; i < range_count; i++) {
        if (merged_count > 0 &&
            ranges[merged_count - 1].width == ranges[i].width &&
            ranges[merged_count - 1].end + 1 == ranges[i].start) {
            // Adjacent and same width: merge
            ranges[merged_count - 1].end = ranges[i].end;
            ranges[merged_count - 1].count += ranges[i].count;
        } else {
            // Not mergeable: copy to output
            ranges[merged_count] = ranges[i];
            merged_count++;
        }
    }
    range_count = merged_count;

    printf("After merging: %d ranges.\n\n", range_count);

    printf("Start     End       Count     Width\n");
    printf("--------- --------- --------- -----\n");

    for (int i = 0; i < range_count; i++) {
        uint32_t count = ranges[i].end - ranges[i].start + 1;
        printf("0x%06X   0x%06X   %7u   %3d\n",
               ranges[i].start, ranges[i].end, count, ranges[i].width);
    }

    return 0;
}

/*
Scanning wcwidth() for target ranges...

Scanning ASCII (U+0000–U+007F)...
Scanning Latin-1 Supplement (U+0080–U+00FF)...
Scanning Latin Extended-A (U+0100–U+017F)...
Scanning Latin Extended-B (U+0180–U+024F)...
Scanning Superscripts (U+2070–U+2079)...
Scanning Mathematical Operators (U+2200–U+22FF)...
Scanning Combining Diacritical Marks (U+0300–U+036F)...
Scanning Greek (U+0370–U+03FF)...
Scanning Cyrillic (U+0400–U+04FF)...
Scanning Cyrillic Supplement (U+0500–U+052F)...
Scanning Hiragana (U+3040–U+309F)...
Scanning Katakana (U+30A0–U+30FF)...
Scanning CJK Unified Ideographs Extension A (U+3400–U+4DBF)...
Scanning CJK Unified Ideographs (U+4E00–U+9FFF)...
Scanning Hangul Syllables (U+AC00–U+D7A3)...
Scanning Hangul Jamo (U+1100–U+11FF)...
Scanning General Punctuation (U+2000–U+206F)...
Scanning Variation Selectors (U+FE00–U+FE0F)...
Done. Found 50 ranges.

After filtering out -1 results: 37 ranges.

After merging: 32 ranges.

Start     End       Count     Width
--------- --------- --------- -----
0x000000   0x000000         1     0
0x000020   0x00007E        95     1
0x0000A0   0x0000AC        13     1
0x0000AE   0x00024F       418     1
0x002070   0x002071         2     1
0x002074   0x002079         6     1
0x002200   0x0022FF       256     1
0x000300   0x00036F       112     0
0x000370   0x000377         8     1
0x00037A   0x00037F         6     1
0x000384   0x00038A         7     1
0x00038C   0x00038C         1     1
0x00038E   0x0003A1        20     1
0x0003A3   0x000482       224     1
0x000483   0x000489         7     0
0x00048A   0x00052F       166     1
0x003041   0x003096        86     2
0x003099   0x00309A         2     0
0x00309B   0x0030FF       101     2
0x003400   0x004DBF      6592     2
0x004E00   0x009FFF     20992     2
0x00AC00   0x00D7A3     11172     2
0x001100   0x00115F        96     2
0x001160   0x0011FF       160     0
0x002000   0x00200A        11     1
0x00200B   0x00200F         5     0
0x002010   0x002027        24     1
0x002028   0x00202E         7     0
0x00202F   0x00205F        49     1
0x002060   0x002064         5     0
0x002066   0x00206F        10     0
0x00FE00   0x00FE0F        16     0
*/
