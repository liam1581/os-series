#include <stdint.h>
#include <stddef.h>

#include "string.h"
#include "print.h"
#include "timer.h"
#include "lse.h"
#include "x86_64/power.h"
#include "x86_64/serial.h"
#include "x86_64/iso9660.h"
#include "x86_64/atapi.h"


void cmd_echo(const char* input) {
    const char* args = input + 5;

    if (args[0] != '"') {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("echo: expected a quoted string, e.g. echo \"hello\"");
        
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    size_t start = 1;
    size_t end = start;
    while (args[end] != '"' && args[end] != '\0') {
        end++;
    }

    if (args[end] == '\0') {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("echo: missing closing quote");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    for (size_t i = start; i < end; i++) {
        printc(args[i]);
    }
    printc('\n');
}

void cmd_help() {
    println("Available commands:");
    println("  help - Show this message");
    println("  echo \"message\" - Print a message");
    println("  cls - Clear the screen");
    println("  reboot - Restart the computer");
    println("  shutdown - Shut down the computer (QEMU & VIRTUALBOX ONLY, NO ACPI)");
    println("  serial.init \"baudrate\" - Init serial port with the baudrate");
    println("  serial.write \"message\" - Write a message to the serial port");
    println("  serial.kill - Close the serial port connection");

    
    println("Available keyboard shortcuts:");
    println("  Ctrl + Alt + E - Toggle command/text mode");
    println("  Ctrl + Alt + C - Clear the screen");
    println("  Ctrl + Alt + R - Restart the computer");
    println("  Ctrl + Alt + S - Shut down the computer (QEMU & VIRTUALBOX ONLY, NO ACPI)");
}

void cmd_cls() {
    clear_screen();
    move_cursor_to_start();
}

void cmd_restart() {
    clear_screen();
    move_cursor_to_start();
    println("Restarting...");
    delay_s(2);
    arch_restart();
}

void cmd_shutdown() {
    clear_screen();
    move_cursor_to_start();
    println("Shutting down...");
    delay_s(2);
    arch_shutdown();
}

void cmd_serial_init(const char* input) {
    const char* args = input + 12;

    if (args[0] != '"') {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("serial.init: expected a quoted string, e.g. serial.init \"115200\"");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    size_t start = 1;
    size_t end = start;
    while (args[end] != '"' && args[end] != '\0') {
        end++;
    }

    if (args[end] == '\0') {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("serial.init: missing closing quote");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    char baud_str[16];
    size_t baud_len = end - start;
    if (baud_len >= sizeof(baud_str)) {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("serial.init: baud rate string too long");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    for (size_t i = 0; i < baud_len; i++) {
        baud_str[i] = args[start + i];
    }
    baud_str[baud_len] = '\0';

    uint32_t baud = (uint32_t)strtoul(baud_str, NULL, 10);
    if (baud == 0) {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("serial.init: invalid baud rate");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    serial_init(baud);
    print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
    print("Serial port initialized at ");
    print_uint64_dec(baud);
    println(" baud.");
    print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    serial_writeln("OS CONNECTED");
}

void cmd_serial_write(const char* input) {
    const char* args = input + 13;

    if (args[0] != '"') {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("serial.write: expected a quoted string, e.g. serial.write \"hello\"");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    size_t start = 1;
    size_t end = start;
    while (args[end] != '"' && args[end] != '\0') {
        end++;
    }

    if (args[end] == '\0') {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("serial.write: missing closing quote");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    for (size_t i = start; i < end; i++) {
        serial_write_byte(args[i]);
    }
    serial_writeln("");
}

bool cdInitialized = false;
char current_dir[1024] = "/data/";

void cmd_atapi_init() {
    if (!atapi_init()) {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("No CD drive found!");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    } else {
        print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
        println("ATAPI CD drive initialized successfully.");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    }
    if (!iso9660_init()) {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("Failed to read ISO filesystem!");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    } else {
        print_set_color(PRINT_COLOR_YELLOW, PRINT_COLOR_BLACK);
        println("ISO9660 filesystem initialized successfully.");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }
    cdInitialized = true;
}

void cmd_ls() {
    if (!cdInitialized) {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("ls: CD drive not initialized. Run 'cd.init' first.");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    println("Listing files in data directory:");
    struct ISO9660Dir dir;
    if (iso9660_list_dir(current_dir, &dir)) {
        for (uint32_t i = 0; i < dir.count; i++) {
            print(dir.entries[i].name);
            if (dir.entries[i].is_directory) print(current_dir);
            printc('\n');
        }
    } else {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("Failed to list dir: Dir not found!");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    }
}

void cmd_cat(const char* input) {
    if (!cdInitialized) {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("cat: CD drive not initialized. Run 'cd.init' first.");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    const char* args = input + 4;

    if (args[0] != '"') {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("cat: expected a quoted string, e.g. cat \"filename.txt\"");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    size_t start = 1;
    size_t end = start;
    while (args[end] != '"' && args[end] != '\0') {
        end++;
    }

    if (args[end] == '\0') {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("cat: missing closing quote");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    char relative_filename[1024];
    size_t filename_len = end - start;
    if (filename_len >= sizeof(relative_filename)) {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("cat: filename too long");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    for (size_t i = 0; i < filename_len; i++) {
        relative_filename[i] = args[start + i];
    }
    relative_filename[filename_len] = '\0';

    char filename[2048];
    strcat_s(filename, current_dir, relative_filename);

    uint8_t file_buf[4096];
    uint32_t file_size;
    if (iso9660_read_file(filename, file_buf, &file_size)) {
        for (uint32_t i = 0; i < file_size; i++) {
            printc((char)file_buf[i]);
        }
        printc('\n');
    } else {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("Failed to read file: File not Found!");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    }
}

void cmd_run(const char* input) {
    if (!cdInitialized) {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("run: CD drive not initialized. Run 'cd.init' first.");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        return;
    }

    const char* args = input + 4;
    if (args[0] != '"') {
        print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
        println("run: expected a quoted path, e.g. run \"/data/example.lse\"");
        print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
    } else {
        size_t start = 1, end = 1;
        while (args[end] != '"' && args[end] != '\0') end++;

        if (args[end] == '\0') {
            print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
            println("run: missing closing quote");
            print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        } else {
            char relative_filename[1024];
            size_t filename_len = end - start;
            if (filename_len >= sizeof(relative_filename)) {
                print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
                println("run: filename too long");
                print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
                return;
            }

            for (size_t i = 0; i < filename_len; i++) {
                relative_filename[i] = args[start + i];
            }
            relative_filename[filename_len] = '\0';

            char path[2048];
            strcat_s(path, current_dir, relative_filename);

            int execStatus = lse_exec(path);

            if (lse_exec(path) != 0) {
                print_set_color(PRINT_COLOR_RED, PRINT_COLOR_BLACK);
                println("run: failed to load or execute program:");
            }

            if (execStatus == -1) {
                println("     File not found!");
            } else if (execStatus == -2) {
                println("     Invalid header size!");
            } else if (execStatus == -3) {
                println("     Invalid header!");
            }
            print_set_color(PRINT_COLOR_WHITE, PRINT_COLOR_BLACK);
        }
    }
}