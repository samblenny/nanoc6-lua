#ifndef UTF8_H
#define UTF8_H

uint32_t utf8_decode_codepoint(const char *buf, int pos, int end);
int utf8_codepoint_width(uint32_t cp);
int utf8_character_width(const char *buf, int pos, int end);

#endif
