#include "keyboard_handler.h"
#include "print.h"
#include "commands.h"

#include "x86_64/ata.h"
#include "x86_64/fat32.h"


void kernel_main() {
    clear_screen();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    println("Welcome to our 64-bit kernel!\n");

    //cmd_atapi_init();

    if (!ata_init()) {
        println("NO ATA DISK FOUND!");
    } else if (!fat32_init()) {
        println("FAT32 init failed!");
    } else {
        println("FAT32 disk ready!");
    }

    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print("> ");

    //keyboard_init();
    //keyboard_set_handler(handle_input);

    while (1);
}