#pragma once
#include <stdint.h>

typedef struct {
    // Print functions
    void (*print)(char* str);
    void (*printc)(char c);
    void (*println)(char* str);
    void (*print_set_color)(uint8_t fg, uint8_t bg);
    void (*clear_screen)();

    // String functions
    int      (*strcmp)(const char* a, const char* b);
    void     (*strcat_s)(char* dest, const char* a, const char* b);
    uint64_t (*strtoul)(const char* str, char** endptr, int base);

    // Serial
    void    (*serial_write)(const char* str);
    void    (*serial_write_byte)(uint8_t byte);
    uint8_t (*serial_read_byte)();

    // Memory
    //void* (*memset)(void* ptr, uint8_t value, uint64_t size);
    //void* (*memcpy)(void* dest, const void* src, uint64_t size);
} KernelAPI;

#define LSE_ENTRY __attribute__((section(".text.entry")))