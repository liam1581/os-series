#pragma once
#include "serial.h"

#ifdef DEBUG
    #define DBG_PRINTS(str) serial_write(str)
#elifndef DEBUG
    #define DBG_PRINTS(str)
#endif
