#include <stdint.h>
#include "../main.h"
#include "ppu.h"
#include "cpu.h"
#include "loader.h"
#include "../logger.h"

uint8_t ppu_memory[PPU_MEMORY_SIZE];
uint8_t oam_memory[OAM_SIZE];
uint8_t oam2_memory[OAM2_SIZE];

ppu_state_t ppu_state;

uint8_t nes_palette[] =
{
    0x7c, 0x7c, 0x7c,
    0x00, 0x00, 0xfc,
    0x00, 0x00, 0xbc,
    0x44, 0x28, 0xbc,
    0x94, 0x00, 0x84,
    0xa8, 0x00, 0x20,
    0xa8, 0x10, 0x00,
    0x88, 0x14, 0x00,
    0x50, 0x30, 0x00,
    0x00, 0x78, 0x00,
    0x00, 0x68, 0x00,
    0x00, 0x58, 0x00,
    0x00, 0x40, 0x58,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0xbc, 0xbc, 0xbc,
    0x00, 0x78, 0xf8,
    0x00, 0x58, 0xf8,
    0x68, 0x44, 0xfc,
    0xd8, 0x00, 0xcc,
    0xe4, 0x00, 0x58,
    0xf8, 0x38, 0x00,
    0xe4, 0x5c, 0x10,
    0xac, 0x7c, 0x00,
    0x00, 0xb8, 0x00,
    0x00, 0xa8, 0x00,
    0x00, 0xa8, 0x44,
    0x00, 0x88, 0x88,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0xf8, 0xf8, 0xf8,
    0x3c, 0xbc, 0xfc,
    0x68, 0x88, 0xfc,
    0x98, 0x78, 0xf8,
    0xf8, 0x78, 0xf8,
    0xf8, 0x58, 0x98,
    0xf8, 0x78, 0x58,
    0xfc, 0xa0, 0x44,
    0xf8, 0xb8, 0x00,
    0xb8, 0xf8, 0x18,
    0x58, 0xd8, 0x54,
    0x58, 0xf8, 0x98,
    0x00, 0xe8, 0xd8,
    0x78, 0x78, 0x78,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
    0xfc, 0xfc, 0xfc,
    0xa4, 0xe4, 0xfc,
    0xb8, 0xb8, 0xf8,
    0xd8, 0xb8, 0xf8,
    0xf8, 0xb8, 0xf8,
    0xf8, 0xa4, 0xc0,
    0xf0, 0xd0, 0xb0,
    0xfc, 0xe0, 0xa8,
    0xf8, 0xd8, 0x78,
    0xd8, 0xf8, 0x78,
    0xb8, 0xf8, 0xb8,
    0xb8, 0xf8, 0xd8,
    0x00, 0xfc, 0xfc,
    0xf8, 0xd8, 0xf8,
    0x00, 0x00, 0x00,
    0x00, 0x00, 0x00,
  };

void ppu_power_up()
{
    ppu_state.cycle = 0;
    ppu_state.scanline = 261; // Start on the pre-scanline

    ppu_state.ctrl = 0;
    ppu_state.mask = 0;
    ppu_state.status = 0;
    ppu_state.oamaddr = 0;
    ppu_state.scroll = 0;
    ppu_state.ppuaddr = 0;
    ppu_state.ppudata = 0;
    ppu_state.oamdma = 0;

    ppu_state.ppuaddr_latch = 0;
    ppu_state.internal_ppu_addr = 0;
    ppu_state.ppuaddr_written = FALSE;
    ppu_state.ppudata_written = FALSE;
    ppu_state.ppuaddr_high = TRUE;
    ppu_state.frame_counter = 0;
    ppu_state.num_sprites = 0;

    Log("PPU powered up", LL_INFO);
}

void perform_next_ppu_cycle()
{
    uint16_t cycle = ppu_state.cycle % 341; // The cycle is 0 - 340 [including]

    if (ppu_state.scanline < 240)
    {
        // The 0th cycle of the ppu is an idle cycle
        if (cycle == 0)
        {
            ppu_state.cycle++;
            return;
        }

        if (cycle > 0 && cycle <= 256)
        {
            uint16_t nametable_base_addr;
            switch (ppu_state.ctrl & NAMETABLE_BITS)
            {
            case 0:
                nametable_base_addr = 0x2000;
                break;
            case 1:
                nametable_base_addr = 0x2400;
                break;
            case 2:
                nametable_base_addr = 0x2800;
                break;
            case 3:
                nametable_base_addr = 0x2C00;
                break;
            }


            // These are the tiles in the nametable
            uint8_t tile_x = (cycle - 1) / 8;
            uint8_t tile_y = ppu_state.scanline / 8;

            // These are the pixel offsets within the tiles
            uint8_t tile_offset_y = ppu_state.scanline % 8;

            // Nametable byte
            ppu_state.nametable_byte = mapper.ppu_read_memory(nametable_base_addr + tile_y * 32 + tile_x);
            // Base address for the pattern table
            uint16_t background_table_addr = ppu_state.ctrl & BC_TILESELECT_BIT ? 0x1000 : 0x0000;
            // Pattern table low byte
            ppu_state.low_pattern_byte = mapper.ppu_read_memory(background_table_addr + ppu_state.nametable_byte * 16 + tile_offset_y);
            // Pattern table high byte
            ppu_state.high_pattern_byte = mapper.ppu_read_memory(background_table_addr + ppu_state.nametable_byte * 16 + 8 + tile_offset_y);

            for (uint8_t tile_offset_x = 0; tile_offset_x < 8; tile_offset_x++)
            {
                uint8_t color = 0;

                // Draw background
                if (BC_ENABLE_BIT & ppu_state.mask)
                {
                    // Every whole tile is within the same attribute area
                    // TODO: this might change when scroll comes into play
                    uint8_t attribute_x = (cycle - 1 + tile_offset_x) / 32;
                    uint8_t attribute_y = ppu_state.scanline / 32;
                    ppu_state.attribute_byte = mapper.ppu_read_memory(nametable_base_addr + NAMETABLE_ATTRIBUTE_OFFSET + attribute_x + attribute_y * 8);


                    uint8_t attribute_offset_x = (cycle - 1 + tile_offset_x) & 0b11111;
                    uint8_t attribute_offset_y = ppu_state.scanline & 0xb11111;
                    uint8_t offset = 7 - tile_offset_x;

                    // The attribute area index of 32x32 area devided into four 16x16
                    //  ----------------
                    //  |  0   |   2   |
                    //  ----------------
                    //  |  4   |   6   |
                    //  ----------------
                    // Index of 2-bit areas in the attribute byte
                    uint8_t attribute_area_index = ((attribute_offset_x >> 4) << 1) + ((attribute_offset_y >> 4) << 2);
                    // Index of the color palette to use
                    uint8_t color_palette_index = (ppu_state.attribute_byte >> attribute_area_index) & 0b11;
                    // Index into the palette (which of the four colors to use)
                    uint8_t color_index = (((ppu_state.high_pattern_byte >> offset) & 1) << 1) + ((ppu_state.low_pattern_byte >> offset) & 1);

                    // Index in the nes-palette
                    color = mapper.ppu_read_memory(PALETTE_ADDRESS + color_palette_index * 4 + color_index);
                }

                // Draw sprites
                if (SPRITE_ENABLE_BIT & ppu_state.mask)
                {
                    for (uint8_t i = 0; i < ppu_state.num_sprites; i++)
                    {
                        // Each sprite takes four bytes
                        // Byte 0: Y position
                        // Byte 1: Tile index number
                        // Byte 2: Attributes
                        // Byte 3: X position

                        uint8_t vpos = oam2_memory[i * 4 + 0];
                        uint8_t tile_index = oam2_memory[i * 4 + 1];
                        uint8_t attribute = oam2_memory[i * 4 + 2];
                        uint8_t hpos = oam2_memory[i * 4 + 3];

                        int16_t offset_y = ppu_state.scanline - vpos;
                        int16_t offset_x = (cycle - 1) + tile_offset_x - hpos;

                        if (offset_x >= 0 && offset_x < 8)
                        {
                            uint8_t tile_bank;
                            // In the case of 8x16 sprite size use the pattern table from the tile index
                            if (ppu_state.ctrl & SPRITE_HIGHT_BIT)
                            {
                                tile_bank = tile_index & OAM_TILE_BANK_BIT;
                            }
                            // In 8x8 sprite mode use the sprite pattern table from the ctrl register
                            else
                            {
                                tile_bank = (ppu_state.ctrl & SPRITE_PT_ADDRESS_BIT) >> 3;
                            }

                            uint16_t tile_addr = (tile_bank << 12) + 16 * (tile_index); // TODO: tile_indez >> 1 in the case of 8x16

                            if (!(attribute & FLIP_H_BIT))
                                offset_x = 7 - offset_x;

                            if (attribute & FLIP_V_BIT)
                                offset_y = 7 - offset_y;

                            uint8_t pattern_low_byte = mapper.ppu_read_memory(tile_addr + 0 + offset_y);
                            uint8_t pattern_high_byte = mapper.ppu_read_memory(tile_addr + 8 + offset_y);

                            uint8_t palette = attribute & PALETTE_BITS;
                            uint8_t color_index = (((pattern_high_byte >> offset_x) & 1) << 1) + ((pattern_low_byte >> offset_x) & 1);

                            // Do not change the color for transparent pixels
                            if (color_index != 0)
                            {
                                color = mapper.ppu_read_memory(PALETTE_ADDRESS + 0x10 + palette * 4 + color_index);
                            }
                        }
                    }
                }

                uint8_t r = nes_palette[color * 3 + 0];
                uint8_t g = nes_palette[color * 3 + 1];
                uint8_t b = nes_palette[color * 3 + 2];

                set_px(cycle + tile_offset_x - 1, ppu_state.scanline, r, g, b);
            }

            ppu_state.cycle += 7;
        }
        // Load sprite data of the next scanline into the secondary oam
        else if (cycle >= 257 && cycle <= 320)
        {
            // Reset the sprite count
            if (cycle == 257)
                ppu_state.num_sprites = 0;

            if (ppu_state.num_sprites < 8)
            {
                // Each sprite takes four bytes, and the sprites are loaded for the next scanline
                // A maximum of 8 sprites can be loaded into the secondary oam for each scanline
                uint16_t next_scanline = ppu_state.scanline;
                uint8_t candidate_index = (cycle - 257) * 4;
                uint8_t vpos = mapper.oam_read(candidate_index);
                int16_t vdelta = next_scanline - vpos;
                if (vdelta >= 0 && vdelta < 8)
                {
                    oam2_memory[ppu_state.num_sprites * 4 + 0] = oam_memory[candidate_index + 0] + 1; // pushing the sprites 1 pixel down
                    oam2_memory[ppu_state.num_sprites * 4 + 1] = oam_memory[candidate_index + 1];
                    oam2_memory[ppu_state.num_sprites * 4 + 2] = oam_memory[candidate_index + 2];
                    oam2_memory[ppu_state.num_sprites * 4 + 3] = oam_memory[candidate_index + 3];
                    ppu_state.num_sprites++;
                }

                // TODO might need to set the sprite overflow flag
            }
        }
    }
    else if (ppu_state.scanline == 240)
    {
        // Just idle
    }
    else // Scanline <= 261
    {
        // Generate NMI and set VBLANK flag if NMI generation is enabled
        if (ppu_state.scanline == 241 && cycle == 1)
        {
            ppu_state.status |= VBLANK;
            if (ppu_state.ctrl & NMI_ENABLE_BIT)
            {
                cpu.nmi_requested = TRUE;
            }
        }
    }

    // Only read and write from VRAM during VBLANK or when rendering is disabled
    if (ppu_state.status & VBLANK || !((SPRITE_ENABLE_BIT | BC_ENABLE_BIT) & ppu_state.mask))
    {
        handle_cpu_vram_reading();
    }

    // Reset the scanline number and VBLANK
    if (ppu_state.scanline == 260 && cycle == 1)
    {
        ppu_state.status &= ~VBLANK;
        ppu_state.frame_counter++;
    }

    ppu_state.cycle++;

    if (cycle == 340)
    {
        ppu_state.scanline = (ppu_state.scanline + 1) % 262; // The scanlines go from 0 - 261 [including]
    }
}

void set_px(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b)
{
    *((PIXEL32 *)(backBuffer.Memory) + (x + ((NES_PX_HEIGHT - y - 1) * NES_PX_WIDTH))) = (PIXEL32){b, g, r, 0};
}

void handle_cpu_vram_reading()
{
    // Handle PPU address writes
    if (ppu_state.ppuaddr_written)
    {
        if (ppu_state.ppuaddr_high)
        {
            ppu_state.ppuaddr_latch = ppu_state.ppuaddr;
            ppu_state.ppuaddr_high = FALSE;
        }
        else
        {
            // The ppuaddr register is already written when this is triggered
            ppu_state.internal_ppu_addr = ((ppu_state.ppuaddr_latch << 8) | ppu_state.ppuaddr) & 0x3FFF;
            ppu_state.ppuaddr_high = TRUE;
        }

        ppu_state.ppuaddr_written = FALSE;
    }
    // Handle PPU data writes
    else if (ppu_state.ppudata_written)
    {
        mapper.ppu_write_memory(ppu_state.internal_ppu_addr, ppu_state.ppudata);

        // If increment mode is set to 0, go across
        if (!(ppu_state.ctrl & INC_MODE_BIT))
        {
            ppu_state.internal_ppu_addr++;
        }
        // If increment mode is set to 1 go down (32)
        else
        {
            ppu_state.internal_ppu_addr += 32;
        }

        ppu_state.ppudata_written = FALSE;
    }
}

void log_ppu_memory()
{
    for (uint16_t i = 0; i < PPU_MEMORY_SIZE / 0x10; i++)
    {
        Logf("%.4x: %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x", LL_DEBUG,
             0x10 * i,
             ppu_memory[0x10 * i + 0x0],
             ppu_memory[0x10 * i + 0x1],
             ppu_memory[0x10 * i + 0x2],
             ppu_memory[0x10 * i + 0x3],
             ppu_memory[0x10 * i + 0x4],
             ppu_memory[0x10 * i + 0x5],
             ppu_memory[0x10 * i + 0x6],
             ppu_memory[0x10 * i + 0x7],
             ppu_memory[0x10 * i + 0x8],
             ppu_memory[0x10 * i + 0x9],
             ppu_memory[0x10 * i + 0xa],
             ppu_memory[0x10 * i + 0xb],
             ppu_memory[0x10 * i + 0xc],
             ppu_memory[0x10 * i + 0xd],
             ppu_memory[0x10 * i + 0xe],
             ppu_memory[0x10 * i + 0xf]);
    }

    Log("OAM:", LL_DEBUG);
    for (uint16_t i = 0; i < OAM_SIZE / 0x10; i++)
    {
        Logf("%.4x: %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x, %.2x", LL_DEBUG,
             0x10 * i,
             oam_memory[0x10 * i + 0x0],
             oam_memory[0x10 * i + 0x1],
             oam_memory[0x10 * i + 0x2],
             oam_memory[0x10 * i + 0x3],
             oam_memory[0x10 * i + 0x4],
             oam_memory[0x10 * i + 0x5],
             oam_memory[0x10 * i + 0x6],
             oam_memory[0x10 * i + 0x7],
             oam_memory[0x10 * i + 0x8],
             oam_memory[0x10 * i + 0x9],
             oam_memory[0x10 * i + 0xa],
             oam_memory[0x10 * i + 0xb],
             oam_memory[0x10 * i + 0xc],
             oam_memory[0x10 * i + 0xd],
             oam_memory[0x10 * i + 0xe],
             oam_memory[0x10 * i + 0xf]);
    }
}