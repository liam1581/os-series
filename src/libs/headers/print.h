#pragma once

#include <stdint.h>
#include <stddef.h>

enum {
    PRINT_COLOR_BLACK = 0,
	PRINT_COLOR_BLUE = 1,
	PRINT_COLOR_GREEN = 2,
	PRINT_COLOR_CYAN = 3,
	PRINT_COLOR_RED = 4,
	PRINT_COLOR_MAGENTA = 5,
	PRINT_COLOR_BROWN = 6,
	PRINT_COLOR_LIGHT_GRAY = 7,
	PRINT_COLOR_DARK_GRAY = 8,
	PRINT_COLOR_LIGHT_BLUE = 9,
	PRINT_COLOR_LIGHT_GREEN = 10,
	PRINT_COLOR_LIGHT_CYAN = 11,
	PRINT_COLOR_LIGHT_RED = 12,
	PRINT_COLOR_PINK = 13,
	PRINT_COLOR_YELLOW = 14,
	PRINT_COLOR_WHITE = 15,
};

void clear_screen();
void printc(char character);
void print(char* string);
void println(char* string);
void printf(char* fmt, ...);
void print_set_color(uint8_t foreground, uint8_t background);
void print_uint64_dec(uint64_t value);
void print_uint64_hex(uint64_t value);
void print_uint64_bin(uint64_t value);
void delete_last_char();

void move_cursor(int row, int col);
void move_cursor_up();
void move_cursor_down();
void move_cursor_left();
void move_cursor_right();
void move_cursor_to_start();