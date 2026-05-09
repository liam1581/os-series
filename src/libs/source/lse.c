#include "lse.h"
#include "x86_64/iso9660.h"
#include "kapi.h"
#include "print.h"
#include "x86_64/serial.h"
#include "string.h"
//#include "memory.h"

// A fixed address in memory to load programs into
// Make sure this doesn't overlap your kernel!
#define LSE_LOAD_ADDRESS 0x400000

static void* kapi_memset(void* ptr, uint8_t value, uint64_t size) {
    uint8_t* p = (uint8_t*)ptr;
    for (uint64_t i = 0; i < size; i++) p[i] = value;
    return ptr;
}

static void* kapi_memcpy(void* dest, const void* src, uint64_t size) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;
    for (uint64_t i = 0; i < size; i++) d[i] = s[i];
    return dest;
}

static bool lse_check_header(const uint8_t* header) {
    return header[0] == LSE_MAGIC_0 &&
           header[1] == LSE_MAGIC_1 &&
           header[2] == LSE_MAGIC_2 &&
           header[3] == LSE_MAGIC_3 &&
           header[4] == LSE_MAGIC_4 &&
           header[5] == LSE_MAGIC_5 &&
           header[6] == LSE_MAGIC_6 &&
           header[7] == LSE_MAGIC_7 &&
           header[8] == LSE_MAGIC_8 &&
           header[9] == LSE_MAGIC_9 &&
           header[10] == LSE_MAGIC_A &&
           header[11] == LSE_MAGIC_B &&
           header[12] == LSE_MAGIC_C &&
           header[13] == LSE_MAGIC_D &&
           header[14] == LSE_MAGIC_E &&
           header[15] == LSE_MAGIC_F;
}

int lse_exec(const char* path) {
    uint8_t* load_addr = (uint8_t*)LSE_LOAD_ADDRESS;

    uint32_t file_size;
    if (!iso9660_read_file(path, load_addr, &file_size)) return -1;

    // Validate header
    if (file_size <= LSE_HEADER_SIZE)  return -2;
    if (!lse_check_header(load_addr))  return -3;

    KernelAPI kapi;
    kapi.print            = print;
    kapi.printc           = printc;
    kapi.println          = println;
    kapi.print_set_color  = print_set_color;
    kapi.clear_screen      = clear_screen;
    kapi.strcmp           = strcmp;
    kapi.strcat_s         = strcat_s;
    kapi.strtoul          = strtoul;
    kapi.serial_write     = serial_write;
    kapi.serial_write_byte = serial_write_byte;
    kapi.serial_read_byte  = serial_read_byte;

    // Jump past the header to the first byte of code and call it as a function
    void (*program_main)(KernelAPI*) = (void(*)(KernelAPI*))(load_addr + LSE_HEADER_SIZE);    
    program_main(&kapi);

    return 1;
}