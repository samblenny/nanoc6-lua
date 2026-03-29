// see docs at
// https://docs.espressif.com/projects/esp-idf/en/v6.0/esp32c6/api-guides/unit-tests.html

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

TEST_CASE("decode 3-byte codepoint: U+4f60", "[utf8]")
{
    uint32_t n = utf8_decode_codepoint("你", 0, 3);
    TEST_ASSERT_EQUAL(0x00004f60, n);
}

TEST_CASE("decode 4-byte codepoint: U+1f600", "[utf8]")
{
    uint32_t n = utf8_decode_codepoint("😀", 0, 4);
    TEST_ASSERT_EQUAL(0x0001f600, n);
}
