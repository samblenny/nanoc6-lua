// see docs at
// https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32c6/api-guides/unit-tests.html

#include <string.h>
#include "unity.h"
#include "utf8.h"

TEST_CASE("decode 1-byte codepoint: U+41", "[utf8]")
{
    uint32_t n = utf8_decode_codepoint("A", 0, 1);
    TEST_ASSERT_EQUAL(0x00000041, n);
}

TEST_CASE("decode 2-byte codepoint: U+3c9", "[utf8]")
{
    uint32_t n = utf8_decode_codepoint("ω", 0, 2);
    TEST_ASSERT_EQUAL(0x000003c9, n);
}

TEST_CASE("decode 3-byte codepoint: U+597d", "[utf8]")
{
    uint32_t n = utf8_decode_codepoint("好", 0, 3);
    TEST_ASSERT_EQUAL(0x0000597d, n);
}

TEST_CASE("decode 4-byte codepoint: U+1f600", "[utf8]")
{
    uint32_t n = utf8_decode_codepoint("😀", 0, 4);
    TEST_ASSERT_EQUAL(0x0001f600, n);
}

TEST_CASE("ASCII character width", "[utf8]")
{
    int n = utf8_codepoint_width("A", 0, 1);
    TEST_ASSERT_EQUAL(1, n);
}

// THIS FAILS. wcwidth returns -1. ESP-IDF wcwidth() seems to be ASCII-only,
// and setlocale() apparently only suports a minimal default locale.
TEST_CASE("Chinese character width", "[utf8]")
{
    char *s = "好";
    int n = utf8_codepoint_width(s, 0, strlen(s));
    TEST_ASSERT_EQUAL(2, n);
}

// THIS FAILS. wcwidth returns -1. ESP-IDF wcwidth() seems to be ASCII-only,
// and setlocale() apparently only suports a minimal default locale.
TEST_CASE("Single-codepoint Emoji width", "[utf8]")
{
    char *s = "😀";  // 1F600
    int n = utf8_codepoint_width(s, 0, strlen(s));
    TEST_ASSERT_EQUAL(2, n);
}

// THIS FAILS. wcwidth returns -1. ESP-IDF wcwidth() seems to be ASCII-only,
// and setlocale() apparently only suports a minimal default locale.
TEST_CASE("Multi-codepoint emoji with ZWJ width", "[utf8]")
{
    char *s = "🏴‍☠️";  // 1F3F4 200D 2620 FE0F
    int n = utf8_codepoint_width(s, 0, strlen(s));
    TEST_ASSERT_EQUAL(2, n);
}


/*
==========================================================================
=== Running Unit Tests ===================================================
==========================================================================

Running decode 1-byte codepoint: U+41...
./components/utf8/test/test_utf8.c:8:decode 1-byte codepoint: U+41:PASS
Running decode 2-byte codepoint: U+3c9...
./components/utf8/test/test_utf8.c:14:decode 2-byte codepoint: U+3c9:PASS
Running decode 3-byte codepoint: U+597d...
./components/utf8/test/test_utf8.c:20:decode 3-byte codepoint: U+597d:PASS
Running decode 4-byte codepoint: U+1f600...
./components/utf8/test/test_utf8.c:26:decode 4-byte codepoint: U+1f600:PASS
Running ASCII character width...
./components/utf8/test/test_utf8.c:32:ASCII character width:PASS
Running Chinese character width...
./components/utf8/test/test_utf8.c:44:Chinese character width:FAIL: Expected 2 Was -1. Function [utf8]
Running Single-codepoint Emoji width...
./components/utf8/test/test_utf8.c:53:Single-codepoint Emoji width:FAIL: Expected 2 Was -1. Function [utf8]
Running Multi-codepoint emoji with ZWJ width...
./components/utf8/test/test_utf8.c:62:Multi-codepoint emoji with ZWJ width:FAIL: Expected 2 Was -1. Function [utf8]

-----------------------
8 Tests 3 Failures 0 Ignored
FAIL
*/
