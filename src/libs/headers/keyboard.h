#pragma once
#include <stdint.h>

#include "bool.h"
#include "keycodes.h"

enum {
	KEYBOARD_EVENT_TYPE_MAKE = 0,
	KEYBOARD_EVENT_TYPE_BREAK = 1,
};

struct KeyboardEvent {
	uint8_t type;
	uint16_t code;
};

void keyboard_init();
void keyboard_set_handler(void (*handler)(struct KeyboardEvent event));
bool keyboard_is_down(uint16_t code);
bool keyboard_is_up(uint16_t code);

char keycode_to_ascii(uint16_t code);
char keycode_to_ascii_ext(uint16_t code, bool shift_pressed, bool altgr_pressed);