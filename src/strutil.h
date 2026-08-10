#ifndef WINDOS_STRUTIL_H
#define WINDOS_STRUTIL_H

#include "vga.h" /* pour les types de base uint8_t/bool_t/TRUE/FALSE */

int str_len(const char* s);
bool_t str_eq(const char* a, const char* b);
bool_t str_starts_with(const char* s, const char* prefix);
void str_copy(char* dest, const char* src, int max_len);
int str_tokenize(char* line, char* argv[], int max_args);
bool_t str_to_int(const char* s, int* out);

#endif
