#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include "cpu.h"
#include "ppu.h"
#include "../logger.h"
#include "loader.h"

nes_cpu cpu;
uint8_t cpu_memory[CPU_MEMORY_SIZE] = {0};

instruction_t instruction_set[] =
    {
        {BRK, ADDR_IMPLIED, 1, 7},     // 0x00
        {ORA, ADDR_X_INDIRECT, 2, 6},  // 0x01
        {NIL, NO_ADDR, 0, 0},          // 0x02
        {NIL, NO_ADDR, 0, 0},          // 0x03
        {NIL, NO_ADDR, 0, 0},          // 0x04
        {ORA, ADDR_ZEROPAGE, 2, 3},    // 0x05
        {ASL, ADDR_ZEROPAGE, 2, 5},    // 0x06
        {NIL, NO_ADDR, 0, 0},          // 0x07
        {PHP, ADDR_IMPLIED, 1, 3},     // 0x08
        {ORA, ADDR_IMMEDIATE, 2, 2},   // 0x09
        {ASL, ADDR_ACCUMULATOR, 1, 2}, // 0x0A
        {NIL, NO_ADDR, 0, 0},          // 0x0B
        {NIL, NO_ADDR, 0, 0},          // 0x0C
        {ORA, ADDR_ABSOLUTE, 3, 4},    // 0x0D
        {ASL, ADDR_ABSOLUTE, 3, 6},    // 0x0E
        {NIL, NO_ADDR, 0, 0},          // 0x0F

        {BPL, ADDR_RELATIVE, 2, 2},   // 0x10 **
        {ORA, ADDR_INDIRECT_Y, 2, 5}, // 0x11 *
        {NIL, NO_ADDR, 0, 0},         // 0x12
        {NIL, NO_ADDR, 0, 0},         // 0x13
        {NIL, NO_ADDR, 0, 0},         // 0x14
        {ORA, ADDR_ZEROPAGE_X, 2, 4}, // 0x15
        {ASL, ADDR_ZEROPAGE_X, 2, 6}, // 0x16
        {NIL, NO_ADDR, 0, 0},         // 0x17
        {CLC, ADDR_IMPLIED, 1, 2},    // 0x18
        {ORA, ADDR_ABSOLUTE_Y, 3, 4}, // 0x19 *
        {NIL, NO_ADDR, 0, 0},         // 0x1A
        {NIL, NO_ADDR, 0, 0},         // 0x1B
        {NIL, NO_ADDR, 0, 0},         // 0x1C
        {ORA, ADDR_ABSOLUTE_X, 3, 4}, // 0x1D *
        {ASL, ADDR_ABSOLUTE_X, 3, 7}, // 0x1E
        {NIL, NO_ADDR, 0, 0},         // 0x1F

        {JSR, ADDR_ABSOLUTE, 3, 6},    // 0x20
        {AND, ADDR_X_INDIRECT, 2, 6},  // 0x21
        {NIL, NO_ADDR, 0, 0},          // 0x22
        {NIL, NO_ADDR, 0, 0},          // 0x23
        {BIT, ADDR_ZEROPAGE, 2, 3},    // 0x24
        {AND, ADDR_ZEROPAGE, 2, 3},    // 0x25
        {ROL, ADDR_ZEROPAGE, 2, 5},    // 0x26
        {NIL, NO_ADDR, 0, 0},          // 0x27
        {PLP, ADDR_IMPLIED, 1, 4},     // 0x28
        {AND, ADDR_IMMEDIATE, 2, 2},   // 0x29
        {ROL, ADDR_ACCUMULATOR, 1, 2}, // 0x2A
        {NIL, NO_ADDR, 0, 0},          // 0x2B
        {BIT, ADDR_ABSOLUTE, 3, 4},    // 0x2C
        {AND, ADDR_ABSOLUTE, 3, 4},    // 0x2D
        {ROL, ADDR_ABSOLUTE, 3, 6},    // 0x2E
        {NIL, NO_ADDR, 0, 0},          // 0x2F

        {BMI, ADDR_RELATIVE, 2, 2},   // 0x30 **
        {AND, ADDR_INDIRECT_Y, 2, 5}, // 0x31 *
        {NIL, NO_ADDR, 0, 0},         // 0x32
        {NIL, NO_ADDR, 0, 0},         // 0x33
        {NIL, NO_ADDR, 0, 0},         // 0x34
        {AND, ADDR_ZEROPAGE_X, 2, 4}, // 0x35
        {ROL, ADDR_ZEROPAGE_X, 2, 6}, // 0x36
        {NIL, NO_ADDR, 0, 0},         // 0x37
        {SEC, ADDR_IMPLIED, 1, 2},    // 0x38
        {AND, ADDR_ABSOLUTE_Y, 3, 4}, // 0x39 *
        {NIL, NO_ADDR, 0, 0},         // 0x3A
        {NIL, NO_ADDR, 0, 0},         // 0x3B
        {NIL, NO_ADDR, 0, 0},         // 0x3C
        {AND, ADDR_ABSOLUTE_X, 3, 4}, // 0x3D *
        {ROL, ADDR_ABSOLUTE_X, 3, 7}, // 0x3E
        {NIL, NO_ADDR, 0, 0},         // 0x3F

        {RTI, ADDR_IMPLIED, 1, 6},     // 0x40
        {EOR, ADDR_X_INDIRECT, 2, 6},  // 0x41
        {NIL, NO_ADDR, 0, 0},          // 0x42
        {NIL, NO_ADDR, 0, 0},          // 0x43
        {NIL, NO_ADDR, 0, 0},          // 0x44
        {EOR, ADDR_ZEROPAGE, 2, 3},    // 0x45
        {LSR, ADDR_ZEROPAGE, 2, 5},    // 0x46
        {NIL, NO_ADDR, 0, 0},          // 0x47
        {PHA, ADDR_IMPLIED, 1, 3},     // 0x48
        {EOR, ADDR_IMMEDIATE, 2, 2},   // 0x49
        {LSR, ADDR_ACCUMULATOR, 1, 2}, // 0x4A
        {NIL, NO_ADDR, 0, 0},          // 0x4B
        {JMP, ADDR_ABSOLUTE, 3, 3},    // 0x4C
        {EOR, ADDR_ABSOLUTE, 3, 4},    // 0x4D
        {LSR, ADDR_ABSOLUTE, 3, 6},    // 0x4E
        {NIL, NO_ADDR, 0, 0},          // 0x4F

        {BVC, ADDR_RELATIVE, 2, 2},   // 0x50 **
        {EOR, ADDR_INDIRECT_Y, 2, 5}, // 0x51 *
        {NIL, NO_ADDR, 0, 0},         // 0x52
        {NIL, NO_ADDR, 0, 0},         // 0x53
        {NIL, NO_ADDR, 0, 0},         // 0x54
        {EOR, ADDR_ZEROPAGE_X, 2, 4}, // 0x55
        {LSR, ADDR_ZEROPAGE_X, 2, 6}, // 0x56
        {NIL, NO_ADDR, 0, 0},         // 0x57
        {CLI, ADDR_IMPLIED, 1, 2},    // 0x58
        {EOR, ADDR_ABSOLUTE_Y, 3, 4}, // 0x59 *
        {NIL, NO_ADDR, 0, 0},         // 0x5A
        {NIL, NO_ADDR, 0, 0},         // 0x5B
        {NIL, NO_ADDR, 0, 0},         // 0x5C
        {EOR, ADDR_ABSOLUTE_X, 3, 4}, // 0x5D *
        {LSR, ADDR_ABSOLUTE_X, 3, 7}, // 0x5E
        {NIL, NO_ADDR, 0, 0},         // 0x5F

        {RTS, ADDR_IMPLIED, 1, 6},     // 0x60
        {ADC, ADDR_X_INDIRECT, 2, 6},  // 0x61
        {NIL, NO_ADDR, 0, 0},          // 0x62
        {NIL, NO_ADDR, 0, 0},          // 0x63
        {NIL, NO_ADDR, 0, 0},          // 0x64
        {ADC, ADDR_ZEROPAGE, 2, 3},    // 0x65
        {ROR, ADDR_ZEROPAGE, 2, 5},    // 0x66
        {NIL, NO_ADDR, 0, 0},          // 0x67
        {PLA, ADDR_IMPLIED, 1, 4},     // 0x68
        {ADC, ADDR_IMMEDIATE, 2, 2},   // 0x69
        {ROR, ADDR_ACCUMULATOR, 1, 2}, // 0x6A
        {NIL, NO_ADDR, 0, 0},          // 0x6B
        {JMP, ADDR_INDIRECT, 3, 5},    // 0x6C
        {ADC, ADDR_ABSOLUTE, 3, 4},    // 0x6D
        {ROR, ADDR_ABSOLUTE, 3, 6},    // 0x6E
        {NIL, NO_ADDR, 0, 0},          // 0x6F

        {BVS, ADDR_RELATIVE, 2, 2},   // 0x70 **
        {ADC, ADDR_INDIRECT_Y, 2, 5}, // 0x71 *
        {NIL, NO_ADDR, 0, 0},         // 0x72
        {NIL, NO_ADDR, 0, 0},         // 0x73
        {NIL, NO_ADDR, 0, 0},         // 0x74
        {ADC, ADDR_ZEROPAGE_X, 2, 4}, // 0x75
        {ROR, ADDR_ZEROPAGE_X, 2, 6}, // 0x76
        {NIL, NO_ADDR, 0, 0},         // 0x77
        {SEI, ADDR_IMPLIED, 1, 2},    // 0x78
        {ADC, ADDR_ABSOLUTE_Y, 3, 4}, // 0x79 *
        {NIL, NO_ADDR, 0, 0},         // 0x7A
        {NIL, NO_ADDR, 0, 0},         // 0x7B
        {NIL, NO_ADDR, 0, 0},         // 0x7C
        {ADC, ADDR_ABSOLUTE_X, 3, 4}, // 0x7D *
        {ROR, ADDR_ABSOLUTE_X, 3, 7}, // 0x7E
        {NIL, NO_ADDR, 0, 0},         // 0x7F

        {NIL, NO_ADDR, 0, 0},         // 0x80
        {STA, ADDR_X_INDIRECT, 2, 6}, // 0x81
        {NIL, NO_ADDR, 0, 0},         // 0x82
        {NIL, NO_ADDR, 0, 0},         // 0x83
        {STY, ADDR_ZEROPAGE, 2, 3},   // 0x84
        {STA, ADDR_ZEROPAGE, 2, 3},   // 0x85
        {STX, ADDR_ZEROPAGE, 2, 3},   // 0x86
        {NIL, NO_ADDR, 0, 0},         // 0x87
        {DEY, ADDR_IMPLIED, 1, 2},    // 0x88
        {NIL, NO_ADDR, 0, 0},         // 0x89
        {TXA, ADDR_IMPLIED, 1, 2},    // 0x8A
        {NIL, NO_ADDR, 0, 0},         // 0x8B
        {STY, ADDR_ABSOLUTE, 3, 4},   // 0x8C
        {STA, ADDR_ABSOLUTE, 3, 4},   // 0x8D
        {STX, ADDR_ABSOLUTE, 3, 4},   // 0x8E
        {NIL, NO_ADDR, 0, 0},         // 0x8F

        {BCC, ADDR_RELATIVE, 2, 2},   // 0x90 **
        {STA, ADDR_INDIRECT_Y, 2, 6}, // 0x91
        {NIL, NO_ADDR, 0, 0},         // 0x92
        {NIL, NO_ADDR, 0, 0},         // 0x93
        {STY, ADDR_ZEROPAGE_X, 2, 4}, // 0x94
        {STA, ADDR_ZEROPAGE_X, 2, 4}, // 0x95
        {STX, ADDR_ZEROPAGE_Y, 2, 4}, // 0x96
        {NIL, NO_ADDR, 0, 0},         // 0x97
        {TYA, ADDR_IMPLIED, 1, 2},    // 0x98
        {STA, ADDR_ABSOLUTE_Y, 3, 5}, // 0x99
        {TXS, ADDR_IMPLIED, 1, 2},    // 0x9A
        {NIL, NO_ADDR, 0, 0},         // 0x9B
        {NIL, NO_ADDR, 0, 0},         // 0x9C
        {STA, ADDR_ABSOLUTE_X, 3, 5}, // 0x9D
        {NIL, NO_ADDR, 0, 0},         // 0x9E
        {NIL, NO_ADDR, 0, 0},         // 0x9F

        {LDY, ADDR_IMMEDIATE, 2, 2},  // 0xA0
        {LDA, ADDR_X_INDIRECT, 2, 6}, // 0xA1
        {LDX, ADDR_IMMEDIATE, 2, 2},  // 0xA2
        {NIL, NO_ADDR, 0, 0},         // 0xA3
        {LDY, ADDR_ZEROPAGE, 2, 3},   // 0xA4
        {LDA, ADDR_ZEROPAGE, 2, 3},   // 0xA5
        {LDX, ADDR_ZEROPAGE, 2, 3},   // 0xA6
        {NIL, NO_ADDR, 0, 0},         // 0xA7
        {TAY, ADDR_IMPLIED, 1, 2},    // 0xA8
        {LDA, ADDR_IMMEDIATE, 2, 2},  // 0xA9
        {TAX, ADDR_IMPLIED, 1, 2},    // 0xAA
        {NIL, NO_ADDR, 0, 0},         // 0xAB
        {LDY, ADDR_ABSOLUTE, 3, 4},   // 0xAC
        {LDA, ADDR_ABSOLUTE, 3, 4},   // 0xAD
        {LDX, ADDR_ABSOLUTE, 3, 4},   // 0xAE
        {NIL, NO_ADDR, 0, 0},         // 0xAF

        {BCS, ADDR_RELATIVE, 2, 2},   // 0xB0 **
        {LDA, ADDR_INDIRECT_Y, 2, 5}, // 0xB1 *
        {NIL, NO_ADDR, 0, 0},         // 0xB2
        {NIL, NO_ADDR, 0, 0},         // 0xB3
        {LDY, ADDR_ZEROPAGE_X, 2, 4}, // 0xB4
        {LDA, ADDR_ZEROPAGE_X, 2, 4}, // 0xB5
        {LDX, ADDR_ZEROPAGE_Y, 2, 4}, // 0xB6
        {NIL, NO_ADDR, 0, 0},         // 0xB7
        {CLV, ADDR_IMPLIED, 1, 2},    // 0xB8
        {LDA, ADDR_ABSOLUTE_Y, 3, 4}, // 0xB9 *
        {TSX, ADDR_IMPLIED, 1, 2},    // 0xBA
        {NIL, NO_ADDR, 0, 0},         // 0xBB
        {LDY, ADDR_ABSOLUTE_X, 3, 4}, // 0xBC *
        {LDA, ADDR_ABSOLUTE_X, 3, 4}, // 0xBD *
        {LDX, ADDR_ABSOLUTE_Y, 3, 4}, // 0xBE *
        {NIL, NO_ADDR, 0, 0},         // 0xBF

        {CPY, ADDR_IMMEDIATE, 2, 2},  // 0xC0
        {CMP, ADDR_X_INDIRECT, 2, 6}, // 0xC1
        {NIL, NO_ADDR, 0, 0},         // 0xC2
        {NIL, NO_ADDR, 0, 0},         // 0xC3
        {CPY, ADDR_ZEROPAGE, 2, 3},   // 0xC4
        {CMP, ADDR_ZEROPAGE, 2, 3},   // 0xC5
        {DEC, ADDR_ZEROPAGE, 2, 5},   // 0xC6
        {NIL, NO_ADDR, 0, 0},         // 0xC7
        {INY, ADDR_IMPLIED, 1, 2},    // 0xC8
        {CMP, ADDR_IMMEDIATE, 2, 2},  // 0xC9 *
        {DEX, ADDR_IMPLIED, 1, 2},    // 0xCA
        {NIL, NO_ADDR, 0, 0},         // 0xCB
        {CPY, ADDR_ABSOLUTE, 3, 4},   // 0xCC
        {CMP, ADDR_ABSOLUTE, 3, 4},   // 0xCD
        {DEC, ADDR_ABSOLUTE, 3, 3},   // 0xCE
        {NIL, NO_ADDR, 0, 0},         // 0xCF

        {BNE, ADDR_RELATIVE, 2, 2},   // 0xD0 **
        {CMP, ADDR_INDIRECT_Y, 2, 5}, // 0xD1 *
        {NIL, NO_ADDR, 0, 0},         // 0xD2
        {NIL, NO_ADDR, 0, 0},         // 0xD3
        {NIL, NO_ADDR, 0, 0},         // 0xD4
        {CMP, ADDR_ZEROPAGE_X, 2, 4}, // 0xD5
        {DEC, ADDR_ZEROPAGE_X, 2, 6}, // 0xD6
        {NIL, NO_ADDR, 0, 0},         // 0xD7
        {CLD, ADDR_IMPLIED, 1, 2},    // 0xD8
        {CMP, ADDR_ABSOLUTE_Y, 3, 4}, // 0xD9 *
        {NIL, NO_ADDR, 0, 0},         // 0xDA
        {NIL, NO_ADDR, 0, 0},         // 0xDB
        {NIL, NO_ADDR, 0, 0},         // 0xDC
        {CMP, ADDR_ABSOLUTE_X, 3, 4}, // 0xDD *
        {DEC, ADDR_ABSOLUTE_X, 3, 7}, // 0xDE
        {NIL, NO_ADDR, 0, 0},         // 0xDF

        {CPX, ADDR_IMMEDIATE, 2, 2},  // 0xE0
        {SBC, ADDR_X_INDIRECT, 2, 6}, // 0xE1
        {NIL, NO_ADDR, 0, 0},         // 0xE2
        {NIL, NO_ADDR, 0, 0},         // 0xE3
        {CPX, ADDR_ZEROPAGE, 2, 3},   // 0xE4
        {SBC, ADDR_ZEROPAGE, 2, 3},   // 0xE5
        {INC, ADDR_ZEROPAGE, 2, 5},   // 0xE6
        {NIL, NO_ADDR, 0, 0},         // 0xE7
        {INX, ADDR_IMPLIED, 1, 2},    // 0xE8
        {SBC, ADDR_IMMEDIATE, 2, 2},  // 0xE9
        {NOP, ADDR_IMPLIED, 1, 2},    // 0xEA
        {NIL, NO_ADDR, 0, 0},         // 0xEB
        {CPX, ADDR_ABSOLUTE, 3, 4},   // 0xEC
        {SBC, ADDR_ABSOLUTE, 3, 4},   // 0xED
        {INC, ADDR_ABSOLUTE, 3, 6},   // 0xEE
        {NIL, NO_ADDR, 0, 0},         // 0xEF

        {BEQ, ADDR_RELATIVE, 2, 2},   // 0xF0 **
        {SBC, ADDR_INDIRECT_Y, 2, 5}, // 0xF1 *
        {NIL, NO_ADDR, 0, 0},         // 0xF2
        {NIL, NO_ADDR, 0, 0},         // 0xF3
        {NIL, NO_ADDR, 0, 0},         // 0xF4
        {SBC, ADDR_ZEROPAGE_X, 2, 5}, // 0xF5
        {INC, ADDR_ZEROPAGE_X, 2, 6}, // 0xF6
        {NIL, NO_ADDR, 0, 0},         // 0xF7
        {SED, ADDR_IMPLIED, 1, 2},    // 0xF8
        {SBC, ADDR_ABSOLUTE_Y, 3, 4}, // 0xF9 *
        {NIL, NO_ADDR, 0, 0},         // 0xFA
        {NIL, NO_ADDR, 0, 0},         // 0xFB
        {NIL, NO_ADDR, 0, 0},         // 0xFC
        {SBC, ADDR_ABSOLUTE_X, 3, 4}, // 0xFD *
        {INC, ADDR_ABSOLUTE_X, 3, 7}, // 0xFE
        {NIL, NO_ADDR, 0, 0},         // 0xFF
};

#define NES_TEST_

void cpu_power_up()
{
    cpu.cycle = 0;
    cpu.nmi_requested = 0;

    // Set registers initial value
    cpu.registers.sr = 0x34;
    cpu.registers.ac = 0x00;
    cpu.registers.x = 0x00;
    cpu.registers.y = 0x00;
    cpu.registers.sp = 0xfd;

    // All channels disabled
    mapper.write_memory(0x4015, 0x00);
    // Frame irq enable
    mapper.write_memory(0x4017, 0x00);

    //memset(&cpu_memory[APU_INPUT_REGISTER_ADDRESS], 0x00, 0x13);
    for (uint8_t i = 0; i < 0x13; i++)
    {
        mapper.write_memory(APU_INPUT_REGISTER_ADDRESS + i, 0);
    }

    cpu.registers.pc = (mapper.read_memory(RESET_VECTOR_ADDRESS + 1) << 8) | mapper.read_memory(RESET_VECTOR_ADDRESS);
    Logf("Initial PC: %x", LL_DEBUG, cpu.registers.pc);

    cpu.powered = TRUE;
}

void perform_next_instruction()
{
    // It is important that an instruction is performed after the interrupt is handled
    // This is due to the fact that and instruction is always performed in an interrupt
    // before other potential interrupts can be handled
    if (cpu.nmi_requested)
        perform_nmi();

    cpu.current_instruction = instruction_set[mapper.read_memory(cpu.registers.pc)];

    /*     if(cpu.registers.pc >= 0xc4b0)
        log_cpu_state(); */

    perform_instruction(cpu.current_instruction);
}

void perform_nmi()
{
    cpu.nmi_requested = FALSE;

    // Push current program counter
    mapper.write_memory(STACK_BASE + cpu.registers.sp, cpu.registers.pc >> 8);
    mapper.write_memory(STACK_BASE + cpu.registers.sp - 1, cpu.registers.pc & 0xff);
    cpu.registers.sp -= 2;

    // Push current status flags and set the B flag
    uint8_t status_flag = cpu.registers.sr;
    SET_5(status_flag, 1);
    SET_B(status_flag, 0);
    mapper.write_memory(STACK_BASE + cpu.registers.sp, status_flag);
    cpu.registers.sp--;

    // Disable interrupt
    SET_I(cpu.registers.sr, 1);

    // Set the program counter to the NMI address
    cpu.registers.pc = (mapper.read_memory(NMI_VECTOR_ADDRESS + 1) << 8) | mapper.read_memory(NMI_VECTOR_ADDRESS);
    cpu.cycle += 2;
}

void perform_oam_dma(uint8_t hbyte)
{
    uint16_t cpu_read_addr = (hbyte << 8) | 0x00;

    for (uint16_t i = 0; i < 256; i++)
    {
        uint8_t value = mapper.read_memory(cpu_read_addr + i);
        mapper.oam_write(i, value);
    }

    cpu.cycle += 514; // TODO this is 513 or 514 depending on odd or even cycle count
}

void perform_instruction(instruction_t instruction)
{
    uint8_t *operand;
    uint8_t immediate;
    // This decides whether the pc should be incremented or not
    BOOL branched = FALSE;
    BOOL jumped = FALSE;

    switch (instruction.addr_mode)
    {
    case ADDR_ACCUMULATOR:
        operand = &cpu.registers.ac;
        break;
    case ADDR_ABSOLUTE:
    {
        uint8_t LL = mapper.read_memory(cpu.registers.pc + 1);
        uint8_t HH = mapper.read_memory(cpu.registers.pc + 2);
        uint16_t address = (HH << 8) | LL;
        operand = mapper.get_memory_pointer(address);
    }
    break;
    case ADDR_ABSOLUTE_X:
    {
        // TODO check page boundery
        uint8_t LL = mapper.read_memory(cpu.registers.pc + 1);
        uint8_t HH = mapper.read_memory(cpu.registers.pc + 2);
        uint16_t address = (((uint16_t)HH) << 8) | LL;
        operand = mapper.get_memory_pointer(address + cpu.registers.x);
    }
    break;
    case ADDR_ABSOLUTE_Y:
    {
        // TODO check page boundery
        uint8_t LL = mapper.read_memory(cpu.registers.pc + 1);
        uint8_t HH = mapper.read_memory(cpu.registers.pc + 2);
        uint16_t address = (((uint16_t)HH) << 8) | LL;
        operand = mapper.get_memory_pointer(address + cpu.registers.y);
    }
    break;
    case ADDR_IMMEDIATE:
        immediate = mapper.read_memory(cpu.registers.pc + 1);
        operand = &immediate;
        break;
    case ADDR_IMPLIED:
        // Each operation has to handle this
        break;
    case ADDR_INDIRECT:
    {
        // JMP is the only operation using this addressing mode
        uint8_t LL = mapper.read_memory(cpu.registers.pc + 1);
        uint8_t HH = mapper.read_memory(cpu.registers.pc + 2);
        // This is important because there is no carry between the low and high byte
        uint8_t LL2 = LL + 1;
        LL = mapper.read_memory((HH << 8) | LL);
        HH = mapper.read_memory((HH << 8) | LL2);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
        operand = (HH << 8) | LL;
#pragma GCC diagnostic pop
    }
    break;
    case ADDR_X_INDIRECT:
    {
        // Using uint8_t is important to not have a carry when incremented by x
        uint8_t zero_page_addr = mapper.read_memory(cpu.registers.pc + 1) + cpu.registers.x;

        uint8_t LL = mapper.read_memory(zero_page_addr);
        uint8_t HH = mapper.read_memory((zero_page_addr + 1) & 0xff);
        uint16_t address = (HH << 8) | LL;
        operand = mapper.get_memory_pointer(address);
    }
    break;
    case ADDR_INDIRECT_Y:
    {
        uint8_t zero_page_addr = mapper.read_memory(cpu.registers.pc + 1);

        uint8_t LL = mapper.read_memory(zero_page_addr);
        uint8_t HH = mapper.read_memory((zero_page_addr + 1) & 0xff);

        uint16_t address = (HH << 8) | LL;
        address += cpu.registers.y;
        operand = mapper.get_memory_pointer(address);
    }
    break;
    case ADDR_RELATIVE:
// TODO Check page boundery
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wint-conversion"
        operand = ((int8_t)mapper.read_memory(cpu.registers.pc + 1)) + cpu.registers.pc;
#pragma GCC diagnostic pop
        break;
    case ADDR_ZEROPAGE:
    {
        uint8_t zero_page_address = mapper.read_memory(cpu.registers.pc + 1);
        operand = mapper.get_memory_pointer(zero_page_address);
    }
    break;
    case ADDR_ZEROPAGE_X:
        // TODO check page boundery
        // Important to not have carry
        {
            uint8_t zero_page_address = mapper.read_memory(cpu.registers.pc + 1) + cpu.registers.x;
            operand = mapper.get_memory_pointer(zero_page_address);
        }
        break;
    case ADDR_ZEROPAGE_Y:
        // TODO check page boundery
        // Important to not have carry
        {
            uint8_t zero_page_address = mapper.read_memory(cpu.registers.pc + 1) + cpu.registers.y;
            operand = mapper.get_memory_pointer(zero_page_address);
        }
        break;
    case NO_ADDR:
        Logf("No-address-mode used: %d", LL_WARNING, instruction.addr_mode);
        break;
    default:
        Logf("Unknown address mode: %d", LL_WARNING, instruction.addr_mode);
    }

    switch (instruction.operation)
    {
    case ADC:
    {

        uint8_t tmp0 = cpu.registers.ac;
        uint16_t res = *operand + cpu.registers.ac + READ_C(cpu.registers.sr);
        cpu.registers.ac = (uint8_t)res;
        SET_V(cpu.registers.sr, ((tmp0 ^ res) & (*operand ^ res)) & 0x80 ? 1 : 0);
        SET_C(cpu.registers.sr, res > 255 ? 1 : 0);
        SET_Z(cpu.registers.sr, (uint8_t)res == 0);
        SET_N(cpu.registers.sr, (uint8_t)res & 0x80 ? 1 : 0);
    }
    break;
    case AND:
    {
        cpu.registers.ac &= *operand;
        SET_Z(cpu.registers.sr, cpu.registers.ac == 0);
        SET_N(cpu.registers.sr, 0b1 & (cpu.registers.ac >> 7));
    }
    break;
    case ASL:
        SET_C(cpu.registers.sr, (*operand >> 7) & 0b1);
        mapper.write_to_pointer(operand, *operand << 1);
        SET_Z(cpu.registers.sr, *operand == 0);
        SET_N(cpu.registers.sr, 0b1 & (*operand >> 7));
        break;
    case BCC:
        if (READ_C(cpu.registers.sr) == 0)
        {
            branched = TRUE;
        }
        break;
    case BCS:
        if (READ_C(cpu.registers.sr) == 1)
        {
            branched = TRUE;
        }
        break;
    case BEQ:
        if (READ_Z(cpu.registers.sr) == 1)
        {
            branched = TRUE;
        }
        break;
    case BIT:
        SET_N(cpu.registers.sr, 0b1 & (*operand >> 7));
        SET_V(cpu.registers.sr, 0b1 & (*operand >> 6));
        SET_Z(cpu.registers.sr, (cpu.registers.ac & *operand) == 0);
        break;
    case BMI:
        if (READ_N(cpu.registers.sr) == 1)
        {
            branched = TRUE;
        }
        break;
    case BNE:
        if (READ_Z(cpu.registers.sr) == 0)
        {
            branched = TRUE;
        }
        break;
    case BPL:
        if (READ_N(cpu.registers.sr) == 0)
        {
            branched = TRUE;
        }
        break;
    case BRK:
    {
        // Write the return address PC + 2 to the stack High byte first
        uint16_t ret_addr = cpu.registers.pc + 2;
        mapper.write_memory(STACK_BASE + cpu.registers.sp, ret_addr >> 8);
        cpu.registers.sp--;
        mapper.write_memory(STACK_BASE + cpu.registers.sp, ret_addr & 0xff);
        cpu.registers.sp--;
        mapper.write_memory(STACK_BASE + cpu.registers.sp, cpu.registers.sr);
        SET_I(*mapper.get_memory_pointer(STACK_BASE + cpu.registers.sp), 1);
        cpu.registers.sp--;

        // Load the new program counter
        cpu.registers.pc = (mapper.read_memory(IRQ_VECTOR_ADDRESS + 1) << 8) | mapper.read_memory(IRQ_VECTOR_ADDRESS);
    }
    break;
    case BVC:
        if (READ_V(cpu.registers.sr) == 0)
        {
            branched = TRUE;
        }
        break;
    case BVS:
        if (READ_V(cpu.registers.sr) == 1)
        {
            branched = TRUE;
        }
        break;
    case CLC:
        SET_C(cpu.registers.sr, 0);
        break;
    case CLD:
        SET_D(cpu.registers.sr, 0);
        break;
    case CLI:
        SET_I(cpu.registers.sr, 0);
        break;
    case CLV:
        SET_V(cpu.registers.sr, 0);
        break;
    case CMP:
        SET_N(cpu.registers.sr, ((cpu.registers.ac - *operand) >> 7) & 0b1);
        SET_Z(cpu.registers.sr, cpu.registers.ac == *operand);
        SET_C(cpu.registers.sr, cpu.registers.ac >= *operand);
        break;
    case CPX:
        SET_N(cpu.registers.sr, ((cpu.registers.x - *operand) >> 7) & 0b1);
        SET_Z(cpu.registers.sr, cpu.registers.x == *operand);
        SET_C(cpu.registers.sr, cpu.registers.x >= *operand);
        break;
    case CPY:
        SET_N(cpu.registers.sr, ((cpu.registers.y - *operand) >> 7) & 0b1);
        SET_Z(cpu.registers.sr, cpu.registers.y == *operand);
        SET_C(cpu.registers.sr, cpu.registers.y >= *operand);
        break;
    case DEC:
        mapper.write_to_pointer(operand, *operand - 1);
        SET_Z(cpu.registers.sr, *operand == 0);
        SET_N(cpu.registers.sr, (*operand >> 7) & 1);
        break;
    case DEX:
        cpu.registers.x--;
        SET_Z(cpu.registers.sr, cpu.registers.x == 0);
        SET_N(cpu.registers.sr, (cpu.registers.x >> 7) & 1);
        break;
    case DEY:
        cpu.registers.y--;
        SET_Z(cpu.registers.sr, cpu.registers.y == 0);
        SET_N(cpu.registers.sr, (cpu.registers.y >> 7) & 1);
        break;
    case EOR:
        cpu.registers.ac ^= *operand;
        SET_Z(cpu.registers.sr, cpu.registers.ac == 0);
        SET_N(cpu.registers.sr, (cpu.registers.ac >> 7) & 1);
        break;
    case INC:
        mapper.write_to_pointer(operand, *operand + 1);
        SET_Z(cpu.registers.sr, *operand == 0);
        SET_N(cpu.registers.sr, (*operand >> 7) & 1);
        break;
    case INX:
        cpu.registers.x++;
        SET_Z(cpu.registers.sr, cpu.registers.x == 0);
        SET_N(cpu.registers.sr, (cpu.registers.x >> 7) & 1);
        break;
    case INY:
        cpu.registers.y++;
        SET_Z(cpu.registers.sr, cpu.registers.y == 0);
        SET_N(cpu.registers.sr, (cpu.registers.y >> 7) & 1);
        break;
    case JMP:
    {
        jumped = TRUE;
        if (cpu.current_instruction.addr_mode == ADDR_ABSOLUTE)
        {
            cpu.registers.pc = (mapper.read_memory(cpu.registers.pc + 2) << 8) | mapper.read_memory(cpu.registers.pc + 1);
        }
        else // ADDR_INDIRECT
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
            cpu.registers.pc = (uint16_t)operand;
#pragma GCC diagnostic pop
        }
    }
    break;
    case JSR:
    {

        jumped = TRUE;
        uint16_t retAddr = cpu.registers.pc + 2;
        mapper.write_memory(STACK_BASE + cpu.registers.sp, retAddr >> 8);
        mapper.write_memory(STACK_BASE + cpu.registers.sp - 1, retAddr & 0xff);
        cpu.registers.sp -= 2;
        cpu.registers.pc = (mapper.read_memory(cpu.registers.pc + 2) << 8) | mapper.read_memory(cpu.registers.pc + 1); // (*(operand + 1) << 8) | *operand;
    }
    break;
    case LDA:
        cpu.registers.ac = mapper.read_pointer(operand);
        SET_N(cpu.registers.sr, (cpu.registers.ac >> 7) & 1);
        SET_Z(cpu.registers.sr, cpu.registers.ac == 0);
        break;
    case LDX:
        cpu.registers.x = mapper.read_pointer(operand);
        SET_N(cpu.registers.sr, (cpu.registers.x >> 7) & 1);
        SET_Z(cpu.registers.sr, cpu.registers.x == 0);
        break;
    case LDY:
        cpu.registers.y = mapper.read_pointer(operand);
        SET_N(cpu.registers.sr, (cpu.registers.y >> 7) & 1);
        SET_Z(cpu.registers.sr, cpu.registers.y == 0);
        break;
    case LSR:
        SET_C(cpu.registers.sr, (*operand & 1));
        mapper.write_to_pointer(operand, *operand >> 1);
        SET_N(cpu.registers.sr, 0);
        SET_Z(cpu.registers.sr, *operand == 0);
        break;
    case NOP:
        // No operation performed
        break;
    case ORA:
        cpu.registers.ac |= *operand;
        SET_N(cpu.registers.sr, (cpu.registers.ac >> 7) & 1);
        SET_Z(cpu.registers.sr, cpu.registers.ac == 0);
        break;
    case PHA:
        mapper.write_memory(STACK_BASE + cpu.registers.sp, cpu.registers.ac);
        cpu.registers.sp--;
        break;
    case PHP:
        mapper.write_memory(STACK_BASE + cpu.registers.sp, cpu.registers.sr);
        SET_5(*mapper.get_memory_pointer(STACK_BASE + cpu.registers.sp), 1);
        SET_B(*mapper.get_memory_pointer(STACK_BASE + cpu.registers.sp), 1);
        cpu.registers.sp--;
        break;
    case PLA:
        cpu.registers.sp++;
        cpu.registers.ac = mapper.read_memory(STACK_BASE + cpu.registers.sp);
        SET_N(cpu.registers.sr, (cpu.registers.ac >> 7) & 1);
        SET_Z(cpu.registers.sr, cpu.registers.ac == 0);
        break;
    case PLP:
        cpu.registers.sp++;
        cpu.registers.sr = mapper.read_memory(STACK_BASE + cpu.registers.sp) & ~((BIT_5 | BIT_B));
        break;
    case ROL:
    {
        bool prev_carry = READ_C(cpu.registers.sr);
        SET_C(cpu.registers.sr, (*operand >> 7) & 1);
        mapper.write_to_pointer(operand, (*operand << 1) | prev_carry);
        SET_N(cpu.registers.sr, (*operand >> 7) & 1);
        SET_Z(cpu.registers.sr, *operand == 0);
    }
    break;
    case ROR:
    {
        bool prev_carry = READ_C(cpu.registers.sr);
        SET_C(cpu.registers.sr, *operand & 1);
        mapper.write_to_pointer(operand, (*operand >> 1) | (prev_carry << 7));
        SET_N(cpu.registers.sr, (*operand >> 7) & 1);
        SET_Z(cpu.registers.sr, *operand == 0);
    }
    break;
    case RTI:
        jumped = TRUE;
        cpu.registers.sp++;
        cpu.registers.sr = mapper.read_memory(STACK_BASE + cpu.registers.sp) & ~((BIT_5 | BIT_I));
        cpu.registers.pc = mapper.read_memory(STACK_BASE + cpu.registers.sp + 1) | (mapper.read_memory(STACK_BASE + cpu.registers.sp + 2) << 8);
        cpu.registers.sp += 2;
        break;
    case RTS:
        cpu.registers.pc = (mapper.read_memory(STACK_BASE + cpu.registers.sp + 1) | (mapper.read_memory(STACK_BASE + cpu.registers.sp + 2) << 8));
        cpu.registers.sp += 2;
        break;
    case SBC:
    {

        uint16_t val = ((uint16_t)*operand) ^ 0x00FF;

        uint16_t tmp = (uint16_t)cpu.registers.ac + val + (uint16_t)READ_C(cpu.registers.sr);

        SET_C(cpu.registers.sr, (tmp & 0xFF00) != 0);
        SET_Z(cpu.registers.sr, (tmp & 0x00FF) == 0);
        SET_V(cpu.registers.sr, ((tmp ^ (uint16_t)cpu.registers.ac) & (tmp ^ val) & 0x0080) != 0);
        SET_N(cpu.registers.sr, (tmp & 0x0080) != 0);

        cpu.registers.ac = tmp & 0x00FF;

        /* uint16_t tmp16 = (uint16_t)cpu.registers.ac - (uint16_t)*operand - (1 - READ_C(cpu.registers.sr));
        uint8_t tmp0 = (uint8_t)tmp16;
        uint8_t tmp1 = cpu.registers.ac;

        cpu.registers.ac = tmp0;

        SET_V(cpu.registers.sr, ((tmp1 ^ tmp0) & 0x80) && (tmp1 ^ *operand) & 0x80);
        SET_C(cpu.registers.sr, tmp16 < 0x100);
        SET_Z(cpu.registers.sr, tmp0 == 0);
        SET_N(cpu.registers.sr, tmp0 & 0x80 ? 1 : 0); */

        /* int16_t twos_comp_sum = (int8_t)cpu.registers.ac + (int8_t) ~*operand - (1 - READ_C(cpu.registers.sr));
        bool overflow = twos_comp_sum > 127 || twos_comp_sum < -128;
        bool carry = (uint16_t)cpu.registers.ac + (uint16_t) ~*operand + (uint16_t)READ_C(cpu.registers.sr) > 0xff;
        cpu.registers.ac += ~*operand - (1 - READ_C(cpu.registers.sr));
        SET_C(cpu.registers.sr, carry);
        SET_V(cpu.registers.sr, overflow);
        SET_Z(cpu.registers.sr, cpu.registers.ac == 0);
        SET_N(cpu.registers.sr, 0b1 & (cpu.registers.ac >> 7)); */
    }
    break;
    case SEC:
        SET_C(cpu.registers.sr, 1);
        break;
    case SED:
        SET_D(cpu.registers.sr, 1);
        break;
    case SEI:
        SET_I(cpu.registers.sr, 1);
        break;
    case STA:
        mapper.write_to_pointer(operand, cpu.registers.ac);
        break;
    case STX:
        mapper.write_to_pointer(operand, cpu.registers.x);
        break;
    case STY:
        mapper.write_to_pointer(operand, cpu.registers.y);
        break;
    case TAX:
        cpu.registers.x = cpu.registers.ac;
        SET_Z(cpu.registers.sr, cpu.registers.x == 0);
        SET_N(cpu.registers.sr, 0b1 & (cpu.registers.x >> 7));
        break;
    case TAY:
        cpu.registers.y = cpu.registers.ac;
        SET_Z(cpu.registers.sr, cpu.registers.y == 0);
        SET_N(cpu.registers.sr, 0b1 & (cpu.registers.y >> 7));
        break;
    case TSX:
        cpu.registers.x = cpu.registers.sp;
        SET_Z(cpu.registers.sr, cpu.registers.x == 0);
        SET_N(cpu.registers.sr, 0b1 & (cpu.registers.x >> 7));
        break;
    case TXA:
        cpu.registers.ac = cpu.registers.x;
        SET_Z(cpu.registers.sr, cpu.registers.ac == 0);
        SET_N(cpu.registers.sr, 0b1 & (cpu.registers.ac >> 7));
        break;
    case TXS:
        cpu.registers.sp = cpu.registers.x;
        break;
    case TYA:
        cpu.registers.ac = cpu.registers.y;
        SET_Z(cpu.registers.sr, cpu.registers.ac == 0);
        SET_N(cpu.registers.sr, 0b1 & (cpu.registers.ac >> 7));
        break;
    case NIL:
        // TODO Remove
        cpu.powered = FALSE;
        Logf("Illegal operation used: %d", LL_WARNING, instruction.operation);
        break;
    default:
        Logf("Unknown operation used: %d", LL_WARNING, instruction.operation);
        break;
    }

    cpu.cycle += cpu.current_instruction.cycles;

    // If the instruction jumps, the program counter does not need to be incremented
    if (!jumped)
    {
        // In the case of branching, the new program counter is set, and then beeing incremented
        // This is because the branch instruction uses relative addresses when branching
        if (branched)
        {
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
            cpu.registers.pc = (uint16_t)operand;
#pragma GCC diagnostic pop
        }

        // In the case of the control flow not being altered, the program counter is incremented to the next instruction
        cpu.registers.pc += instruction.bytes;
    }
}

void log_cpu_mem()
{
    for (uint16_t i = 0; i < CPU_MEMORY_SIZE / 0x10; i++)
    {
        Logf("%.4x: %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x", LL_DEBUG,
             0x10 * i,
             cpu_memory[0x10 * i + 0x0],
             cpu_memory[0x10 * i + 0x1],
             cpu_memory[0x10 * i + 0x2],
             cpu_memory[0x10 * i + 0x3],
             cpu_memory[0x10 * i + 0x4],
             cpu_memory[0x10 * i + 0x5],
             cpu_memory[0x10 * i + 0x6],
             cpu_memory[0x10 * i + 0x7],
             cpu_memory[0x10 * i + 0x8],
             cpu_memory[0x10 * i + 0x9],
             cpu_memory[0x10 * i + 0xa],
             cpu_memory[0x10 * i + 0xb],
             cpu_memory[0x10 * i + 0xc],
             cpu_memory[0x10 * i + 0xd],
             cpu_memory[0x10 * i + 0xe],
             cpu_memory[0x10 * i + 0xf]);
    }
}

void log_cpu_state()
{
    char cpuStatusRegisters[100];
    sprintf(cpuStatusRegisters, "N: %d, V: %d, D: %d, I: %d, Z: %d, C: %d", READ_N(cpu.registers.sr), READ_V(cpu.registers.sr), READ_D(cpu.registers.sr), READ_I(cpu.registers.sr), READ_Z(cpu.registers.sr), READ_C(cpu.registers.sr));

    char cpuRegisters[100];
    sprintf(cpuRegisters, "AC: %.2x X: %.2x Y: %.2x SR: %.2x SP: %.2x", cpu.registers.ac, cpu.registers.x, cpu.registers.y, cpu.registers.sr, cpu.registers.sp);

    char cpuInstruction[100];
    switch (cpu.current_instruction.bytes)
    {
    case 1:
        sprintf(cpuInstruction, "%s\t\t\t", opcode_to_string[cpu.current_instruction.operation]);
        break;
    case 2:
        sprintf(cpuInstruction, "%s %.2x\t\t", opcode_to_string[cpu.current_instruction.operation], mapper.read_memory(cpu.registers.pc + 1));
        break;
    case 3:
        sprintf(cpuInstruction, "%s %.2x, %.2x\t", opcode_to_string[cpu.current_instruction.operation], mapper.read_memory(cpu.registers.pc + 1), mapper.read_memory(cpu.registers.pc + 2));
        break;
    }

    Logf("OPC:%.2x  PC:%.4x\t%s%s\t%s\t CYC: %d\tPPU_CYC: %d\tPPU_LINE:%d", LL_DEBUG, mapper.read_memory(cpu.registers.pc), cpu.registers.pc, cpuInstruction, cpuRegisters, cpuStatusRegisters, cpu.cycle, ppu_state.cycle, ppu_state.scanline);
}