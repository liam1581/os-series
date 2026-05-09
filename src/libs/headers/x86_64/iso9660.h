#pragma once
#include <stdint.h>
#include "bool.h"

#define ISO9660_MAX_FILENAME  32
#define ISO9660_MAX_CHILDREN  64

struct ISO9660Entry {
    char     name[ISO9660_MAX_FILENAME];
    bool     is_directory;
    uint32_t lba;       // starting sector
    uint32_t size;      // file size in bytes
};

struct ISO9660Dir {
    struct ISO9660Entry entries[ISO9660_MAX_CHILDREN];
    uint32_t count;
};

bool     iso9660_init();
bool     iso9660_list_dir(const char* path, struct ISO9660Dir* out);
bool     iso9660_read_file(const char* path, uint8_t* buffer, uint32_t* out_size);