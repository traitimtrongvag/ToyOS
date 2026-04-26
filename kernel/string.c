#include "string.h"

/*
 * string.c - freestanding string/memory utilities
 *
 * No libc. All functions guard against NULL inputs so callers can omit
 * defensive checks on every call site.
 */

void* memset(void* ptr, int value, size_t num) {
    if (!ptr || num == 0) {
        return ptr;
    }
    unsigned char* p = ptr;
    while (num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t n) {
    if (!dest || !src || n == 0) {
        return dest;
    }
    unsigned char* d = dest;
    const unsigned char* s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    if (!s1 || !s2) {
        return 0;
    }
    const unsigned char* p1 = s1;
    const unsigned char* p2 = s2;
    while (n--) {
        if (*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

char* strcpy(char* dest, const char* src) {
    if (!dest || !src) {
        return dest;
    }
    char* d = dest;
    while ((*d++ = *src++));
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    if (!s1 || !s2) {
        return 0;
    }
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}

void* memmove(void* dest, const void* src, size_t n) {
    if (!dest || !src || n == 0) return dest;
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    /* Overlap-safe: copy backwards when dest is inside src region */
    if (d > s && d < s + n) {
        d += n;
        s += n;
        while (n--) *--d = *--s;
    } else {
        while (n--) *d++ = *s++;
    }
    return dest;
}

int strncmp(const char* s1, const char* s2, size_t n) {
    if (!s1 || !s2 || n == 0) return 0;
    while (n-- && *s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    if (n == (size_t)-1) return 0;
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

char* strncpy(char* dest, const char* src, size_t n) {
    if (!dest || !src) return dest;
    size_t i = 0;
    while (i < n && src[i] != '\0') {
        dest[i] = src[i];
        i++;
    }
    /* Pad remaining bytes with NUL per POSIX */
    while (i < n) dest[i++] = '\0';
    return dest;
}

size_t strnlen(const char* str, size_t max) {
    if (!str) return 0;
    size_t len = 0;
    while (len < max && str[len]) len++;
    return len;
}
