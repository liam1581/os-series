#include "keyboard_handler.h"
#include "print.h"
#include "commands.h"


void kernel_main() {
    clear_screen();
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    println("Welcome to our 64-bit kernel!\n");

    cmd_atapi_init();
    cmd_fat_init();

    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    print("> ");

    keyboard_init();
    keyboard_set_handler(handle_input);

    while (1);
}