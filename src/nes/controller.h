#ifndef CONTROLLER_H

#define CONTROLLER_H

#include "Windows.h"
#include "WinUser.h"

#define CONTROLLER_PORT1 0x4016
#define CONTROLLER_PORT2 0x4017

#define STROBE_BIT 0b00000001

#define A_KEYCODE 0x58        // X
#define B_KEYCODE 0x5A        // Z
#define START_KEYCODE VK_RETURN // ENTER
#define SELECT_KEYCODE VK_RSHIFT // SHIFT
#define DPAD_UP VK_UP         // Up arrow
#define DPAD_DOWN VK_DOWN     // Down arrow
#define DPAD_LEFT VK_LEFT     // Left arrow
#define DPAD_RIGHT VK_RIGHT   // Right arrow

typedef union CONTROLLER
{
    struct controller_t
    {
        BOOL a : 1;
        BOOL b : 1;
        BOOL select : 1;
        BOOL start : 1;
        BOOL DPAD_up : 1;
        BOOL DPAD_down : 1;
        BOOL DPAD_left : 1;
        BOOL DPAD_right : 1;
    };

    uint8_t bits:8;
    
} CONTROLLER;

extern CONTROLLER controller;

uint8_t read_controller(uint16_t address);
void write_controller(uint16_t address, uint8_t value);

#endif // CONTROLLER_H