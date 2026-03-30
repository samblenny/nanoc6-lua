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
    int n = utf8_character_width("A", 0, 1);
    TEST_ASSERT_EQUAL(1, n);
}

TEST_CASE("Latin diacritics character width", "[utf8]")
{
    char *chars[5] = {"è", "é", "ë", "ě", "ē"};
    for (int i = 0; i < sizeof(chars)/sizeof(chars[0]); i++) {
        char *s = chars[i];
        int n = utf8_character_width(s, 0, strlen(s));
        TEST_ASSERT_EQUAL(1, n);
    }
}

TEST_CASE("Cyrillic character width", "[utf8]")
{
    char *chars[3] = {"Д", "Є", "Э"};
    for (int i = 0; i < sizeof(chars)/sizeof(chars[0]); i++) {
        char *s = chars[i];
        int n = utf8_character_width(s, 0, strlen(s));
        TEST_ASSERT_EQUAL(1, n);
    }
}

TEST_CASE("Greek character width", "[utf8]")
{
    char *chars[2] = {"∑", "ω"};
    for (int i = 0; i < sizeof(chars)/sizeof(chars[0]); i++) {
        char *s = chars[i];
        int n = utf8_character_width(s, 0, strlen(s));
        TEST_ASSERT_EQUAL(1, n);
    }
}

TEST_CASE("Chinese character width", "[utf8]")
{
    char *s = "好";
    int n = utf8_character_width(s, 0, strlen(s));
    TEST_ASSERT_EQUAL(2, n);
}

TEST_CASE("Single-codepoint Emoji width", "[utf8]")
{
    char *s = "😀";  // 1F600
    int n = utf8_character_width(s, 0, strlen(s));
    TEST_ASSERT_EQUAL(2, n);
}

TEST_CASE("Multi-codepoint emoji with ZWJ width", "[utf8]")
{
    char *s = "🏴‍☠️";  // 1F3F4 200D 2620 FE0F
    int n = utf8_character_width(s, 0, strlen(s));
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
Running Latin diacritics character width...
./components/utf8/test/test_utf8.c:38:Latin diacritics character width:PASS
Running Cyrillic character width...
./components/utf8/test/test_utf8.c:48:Cyrillic character width:PASS
Running Greek character width...
./components/utf8/test/test_utf8.c:59:Greek character width:PASS
Running Chinese character width...
./components/utf8/test/test_utf8.c:69:Chinese character width:PASS
Running Single-codepoint Emoji width...
./components/utf8/test/test_utf8.c:77:Single-codepoint Emoji width:PASS
Running Multi-codepoint emoji with ZWJ width...
./components/utf8/test/test_utf8.c:85:Multi-codepoint emoji with ZWJ width:PASS

-----------------------
11 Tests 0 Failures 0 Ignored
OK
*/
