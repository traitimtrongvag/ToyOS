#ifndef STRING_H
#define STRING_H

#include <stddef.h>
#include <stdint.h>

void* memset(void* ptr, int value, size_t num);
void* memcpy(void* dest, const void* src, size_t n);
void* memmove(void* dest, const void* src, size_t n);
int   memcmp(const void* s1, const void* s2, size_t n);
size_t strlen(const char* str);
size_t strnlen(const char* str, size_t max);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
int   strcmp(const char* s1, const char* s2);
int   strncmp(const char* s1, const char* s2, size_t n);

#endif
