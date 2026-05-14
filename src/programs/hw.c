#include "kapi.h"

#define STR(name, value) char name[] = value

LSE_ENTRY void main(KernelAPI* api) {
    STR(msg, "Hello, World!");
    api->println("Hello");
}