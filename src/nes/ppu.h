#ifndef PPU_H

#define PPU_H

#include "Windows.h"

#define PPU_CTRL_ADDRESS 0x2000
#define PPU_MASK_ADDRESS 0x2001
#define PPU_STATUS_ADDRESS 0x2002
#define OAM_ADDR_ADDRESS 0x2003
#define OAM_DATA_ADDRESS 0x2004
#define PPU_SCROLL_ADDRESS 0x2005
#define PPU_ADDR_ADDRESS 0x2006
#define PPU_DATA_ADDRESS 0x2007
#define OAM_DMA_ADDRESS 0x4014

// Bits for PPUCTRL
#define NMI_ENABLE_BIT 0b10000000
#define PPU_MASTER_BIT 0b01000000
#define SPRITE_HIGHT_BIT 0b00100000
#define BC_TILESELECT_BIT 0b00010000
#define SPRITE_PT_ADDRESS_BIT 0b00001000
#define INC_MODE_BIT 0b00000100
#define NAMETABLE_BITS 0b00000011

// Bits for PPUMASK
#define BRG_BITS 0b11100000
#define SPRITE_ENABLE_BIT 0b00010000
#define BC_ENABLE_BIT 0b00001000
#define SPRITE_LC_ENABLE_BIT 0b00000100
#define BC_LC_ENABLE_BITS 0b00000010
#define GRAYSCALE_BIT 0b00000001

// Bits for PPUSTATUS
#define VBLANK 0b10000000
#define SPRITE_0 0b01000000
#define SPRITE_OVERFLOW 0b00100000

#define PPU_MEMORY_SIZE 0x4000
#define OAM_SIZE 0x100
#define OAM2_SIZE 0x20

#define PATTERN_TABLE_SIZE 0x1000
#define NAME_TABLE_SIZE 0x0400
#define VRAM_ADDRESS 0x2000
#define ATTRIBUTE_TABLE_SIZE 0x40 // This is the last 64 bytes of the nametable
#define NAMETABLE_ATTRIBUTE_OFFSET 0x03C0
#define PALETTE_ADDRESS 0x3F00

// OAM attribute bytes
#define FLIP_V_BIT 0b10000000
#define FLIP_H_BIT 0b01000000
#define PRIORITY_BIT 0b00100000
#define PALETTE_BITS 0b00000011

#define OAM_TILE_BANK_BIT 0b1

typedef struct ppu_state_t
{
    uint64_t cycle;    // The cylces go from 0 to 340
    uint16_t scanline; // The scanlines go from 0 to 240 (260 including)

    uint8_t ctrl;          // 0x2000
    uint8_t mask;          // 0x2001
    uint8_t status;        // 0c2002
    uint8_t oamaddr;       // 0x2003
    uint8_t oamdata;       // 0x2004
    uint8_t scroll;        // 0x2005
    uint8_t ppuaddr;       // 0x2006
    uint8_t ppuaddr_latch; // Upper byte of the ppuaddr
    uint8_t ppudata;       // 0x2007
    uint8_t oamdma;        // 0x4014

    uint8_t nametable_byte;
    uint8_t attribute_byte;
    uint8_t low_pattern_byte;
    uint8_t high_pattern_byte;

    BOOL ppuaddr_high;
    BOOL ppuaddr_written;
    BOOL ppudata_written;

    uint16_t internal_ppu_addr;
    uint8_t num_sprites;
    uint16_t frame_counter;
} ppu_state_t;

extern uint8_t nes_palette[192];
extern ppu_state_t ppu_state;
extern uint8_t ppu_memory[PPU_MEMORY_SIZE];
extern uint8_t oam_memory[OAM_SIZE];
extern uint8_t oam2_memory[OAM2_SIZE];

void ppu_power_up();
void handle_cpu_vram_reading();
void perform_next_ppu_cycle();
void set_px(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
void log_ppu_memory();

#endif