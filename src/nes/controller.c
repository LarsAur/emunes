#include <Windows.h>
#include <stdint.h>
#include "controller.h"
#include "../logger.h"

CONTROLLER controller;
CONTROLLER locked_btn_state;
BOOL strobe = 0;

uint8_t read_controller(uint16_t address)
{
    if(address == CONTROLLER_PORT1)
    {
        if(strobe)
        {
            // It is important to retain the undriven upper bits thus the upper part of the address is retained on the bus
            // When in strobe mode, only the first bit of the controller (btn A) is read
            return 0x40 | (controller.bits & 1); 
        }

        uint8_t ret = 0x80 | (locked_btn_state.bits & 1);
        locked_btn_state.bits >>= 1;
        return ret;
    }
    else if(address == CONTROLLER_PORT2)
    {
        Log("Controller port 2 is not implemented", LL_WARNING);
    }
}

void write_controller(uint16_t address, uint8_t value)
{
    if(address == CONTROLLER_PORT1)
    {
        // Only lock the button state when the stobe is toggled off
        if(strobe && !(value & STROBE_BIT))
        {
            locked_btn_state = controller;
        }

        strobe = value & STROBE_BIT;
    }
}