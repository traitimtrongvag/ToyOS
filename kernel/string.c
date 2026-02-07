#include "string.h"

void* memset(void* ptr, int value, size_t num) {
    unsigned char* p = ptr;
    while(num--) {
        *p++ = (unsigned char)value;
    }
    return ptr;
}

void* memcpy(void* dest, const void* src, size_t n) {
    unsigned char* d = dest;
    const unsigned char* s = src;
    while(n--) {
        *d++ = *s++;
    }
    return dest;
}

int memcmp(const void* s1, const void* s2, size_t n) {
    const unsigned char* p1 = s1;
    const unsigned char* p2 = s2;
    while(n--) {
        if(*p1 != *p2) {
            return *p1 - *p2;
        }
        p1++;
        p2++;
    }
    return 0;
}

char* strcpy(char* dest, const char* src) {
    char* d = dest;
    while((*d++ = *src++));
    return dest;
}

int strcmp(const char* s1, const char* s2) {
    while(*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(unsigned char*)s1 - *(unsigned char*)s2;
}
