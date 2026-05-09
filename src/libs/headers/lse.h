#pragma once
#include <stdint.h>
#include "bool.h"

#define LSE_HEADER_SIZE    16
#define LSE_MAGIC_0        0xFF
#define LSE_MAGIC_1        'L'
#define LSE_MAGIC_2        'S'
#define LSE_MAGIC_3        'O'
#define LSE_MAGIC_4        'S'
#define LSE_MAGIC_5        'F'
#define LSE_MAGIC_6        'H'
#define LSE_MAGIC_7        0x00
#define LSE_MAGIC_8        0x00
#define LSE_MAGIC_9        0x00
#define LSE_MAGIC_A        0x03
#define LSE_MAGIC_B        0x00
#define LSE_MAGIC_C        0x00
#define LSE_MAGIC_D        0x00
#define LSE_MAGIC_E        0x00
#define LSE_MAGIC_F        0xFF

int lse_exec(const char* path);