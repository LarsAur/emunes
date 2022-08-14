#ifndef CPU_H

#define CPU_H

#include "Windows.h"
#include <stdint.h>

#define CYCLES_PER_SEC 1789773
#define CPU_MEMORY_SIZE 0x10000
#define STACK_BASE 0x0100
#define PPU_REGISTER_ADDRESS 0x2000
#define PPU_REGISTER_SIZE 0x2000
#define APU_INPUT_REGISTER_ADDRESS 0x4000
#define PROGRAM_ROM_ADDRESS 0x8000
#define PROGRAM_BANK_SIZE 0x4000 // 16 KB
#define INTERNAL_RAM_BANK_SIZE 0x0800    // 2KB

#define NMI_VECTOR_ADDRESS 0xfffa
#define RESET_VECTOR_ADDRESS 0xfffc
#define IRQ_VECTOR_ADDRESS 0xfffe

// Ref: https://www.masswerk.at/6502/6502_instruction_set.html

#define BIT_N 0b10000000
#define BIT_V 0b01000000
#define BIT_5 0b00100000
#define BIT_B 0b00010000
#define BIT_D 0b00001000
#define BIT_I 0b00000100
#define BIT_Z 0b00000010
#define BIT_C 0b00000001

// Macros for reading and writing flags to the status register
#define READ_N(sr) (sr & 0b10000000) >> 7 // Negative
#define READ_V(sr) (sr & 0b01000000) >> 6 // Overflow
#define READ_B(sr) (sr & 0b00010000) >> 4 // Break
#define READ_D(sr) (sr & 0b00001000) >> 3 // Decimal
#define READ_I(sr) (sr & 0b00000100) >> 2 // Interrupt
#define READ_Z(sr) (sr & 0b00000010) >> 1 // Zero
#define READ_C(sr) (sr & 0b00000001) >> 0 // Carry

#define SET_N(sr, b) sr = (sr & ~0b10000000) | ((b & 0b1) << 7) // Negative
#define SET_V(sr, b) sr = (sr & ~0b01000000) | ((b & 0b1) << 6) // Overflow
#define SET_5(sr, b) sr = (sr & ~0b00100000) | ((b & 0b1) << 5) // Bit 5
#define SET_B(sr, b) sr = (sr & ~0b00010000) | ((b & 0b1) << 4) // Break
#define SET_D(sr, b) sr = (sr & ~0b00001000) | ((b & 0b1) << 3) // Decimal
#define SET_I(sr, b) sr = (sr & ~0b00000100) | ((b & 0b1) << 2) // Interrupt
#define SET_Z(sr, b) sr = (sr & ~0b00000010) | ((b & 0b1) << 1) // Zero
#define SET_C(sr, b) sr = (sr & ~0b00000001) | ((b & 0b1) << 0) // Carry

typedef enum ADDR_MODE
{
    ADDR_ACCUMULATOR, // The operand is the accumulator
    ADDR_ABSOLUTE,    // $LLHH operand is address $HHLL
    ADDR_ABSOLUTE_X,  // $LLHH operand is address $HHLL incremented with carry by the x register
    ADDR_ABSOLUTE_Y,  // $LLHH operand is address $HHLL incremented with carry by the y register
    ADDR_IMMEDIATE,   // Operand is byte $BB
    ADDR_IMPLIED,     // The operand is implied by the operation
    ADDR_INDIRECT,    // ($LLHH) Refers to the value at the address $HHLL
    ADDR_X_INDIRECT,  // ($LL + x, $LL + x + 1) = $LLHH, which is used as an indirect value. The first LL is zero-paged
    ADDR_INDIRECT_Y,  // ($LL, $LL + 1) = $LLHH, this is then incremented by y and used as indirect value.
    ADDR_RELATIVE,    // Branchtarget i PC + signed offset (2's complement)
    ADDR_ZEROPAGE,    // The high byte of the address is zero, thus $00LL
    ADDR_ZEROPAGE_X,  // Address is zero paged incremented by x without carry
    ADDR_ZEROPAGE_Y,  // Address is zero paged incremented by y without carry
    NO_ADDR,          // No address mode, used for illegal instructions
} ADDR_MODE;

typedef enum OPERATION
{
    ADC, // Add with carry
    AND, // And with accumulators
    ASL, // Arithmetic left shift
    BCC, // Branch on carry clear
    BCS, // Branch on carry set
    BEQ, // Branch on equal (zero set)
    BIT, // Bit test
    BMI, // Branch on minus (negative set)
    BNE, // Branch on not equal (zero clear)
    BPL, // Branch on plus (negative clear)
    BRK, // Break / Interrupt
    BVC, // Branch on overflow clear
    BVS, // Branch on overflow set
    CLC, // Clear carry
    CLD, // Clear decimal
    CLI, // Clear interrupt disable
    CLV, // Clear overflow
    CMP, // Compare with accumulator
    CPX, // Compare with x
    CPY, // Compare with y
    DEC, // Decrement
    DEX, // Decrement x
    DEY, // Decrement y
    EOR, // Exclusive or with accumulator
    INC, // Increment
    INX, // Increment x
    INY, // Increment y
    JMP, // Jump
    JSR, // Jump subroutine
    LDA, // Load accumulator
    LDX, // Load x
    LDY, // Load y
    LSR, // Logical shift right
    NOP, // No operation
    ORA, // Or with accumulator
    PHA, // Push accumulator
    PHP, // Push processor status
    PLA, // Pull accumulator
    PLP, // Pull processor status
    ROL, // Rotate left
    ROR, // Rotate right
    RTI, // Return from interrupt
    RTS, // Return form subroutine
    SBC, // Subtract with carry
    SEC, // Set carry
    SED, // Set decimal
    SEI, // Set interrupt disable
    STA, // Store accumulator
    STX, // Store x
    STY, // Store y
    TAX, // Transfer accumulator to x
    TAY, // Transfer accumulator to y
    TSX, // Transfer stack pointer to x
    TXA, // Transfer x to accumulator
    TXS, // Transfer x to stack pointer
    TYA, // Transfer y to accumulator
    NIL, // Illegal instruction
} OPERATION;

typedef struct instruction_t
{
    OPERATION operation;
    ADDR_MODE addr_mode;
    uint8_t bytes;
    uint8_t cycles;
} instruction_t;

typedef struct cpu_registers
{
    uint16_t pc; // Program counter
    uint8_t ac;  // Accumulator
    uint8_t x;   // X register
    uint8_t y;   // Y register
    uint8_t sr;  // Status register
    uint8_t sp;  // Stack pointer
} cpu_registers;

typedef struct nes_cpu
{
    instruction_t current_instruction;
    cpu_registers registers;
    BOOL nmi_requested;
    BOOL powered;
    uint64_t cycle;
} nes_cpu;

extern uint8_t cpu_memory[CPU_MEMORY_SIZE];
extern nes_cpu cpu;
extern instruction_t instruction_set[256];

void perform_next_instruction();
void perform_instruction(instruction_t instruction);
void cpu_power_up();
void perform_nmi();
void log_cpu_mem();
void log_cpu_state();
void perform_oam_dma(uint8_t hbyte);

#endif