#ifndef MAPPER0_H

#include <stdint.h>
#include "../loader.h"
#include "../cpu.h"
#include "../ppu.h"
#include "../controller.h"

uint8_t mapper0_read_memory(uint16_t address)
{
    /*
        The PRG RAM can be of size 2KB, 4KB or 8KB
        in the case of 2KB, all addresses are mapped as such
        0x0000 -> 0x07FF Map
        0x0800 -> 0x0FFF Mirror
        0x1000 -> 0x17FF Mirror
        0x1800 -> 0x1FFF Mirror

        In the case of 4KB, all addresses are mapped as such
        0x0000 -> 0x0FFF Map
        0x1000 -> 0x1FFF Mirror

        In the case of 8KB, all addresses are mapped as such
        0x0000 -> 0x1FFF Map

        TODO: There seems like there only is 8KB in the iNes format
    */
    if (address < INTERNAL_RAM_BANK_SIZE * 4)
    {
        // 8KB internal RAM
        if (header.prg_ram_size == 1)
        {
            // No mirroring is needed
        }
        else
        {
            Logf("Illegal program ram size: %d", LL_WARNING, header.prg_rom_size);
        }

        return cpu_memory[address];
    }

    /*
        The PPU I/O registers at 0x2000 -> 0x2007 are mirrored at every 0x0008 address (8 bytes), thus
        0x2000 -> 0x2007 Map
        0x2008 -> 0x200F Mirror
        0x2010 -> 0x2017 Mirror
        ...
        0x3FF8 -> 0x3FFF Mirror 
    */
    else if (address < PPU_REGISTER_ADDRESS + PPU_REGISTER_SIZE)
    {
        address = PPU_REGISTER_ADDRESS + (address % 0x0008);

        switch (address)
        {
        case PPU_CTRL_ADDRESS:
            Log("Illegal read of PPU Ctrl register", LL_WARNING);
            return ppu_state.ctrl; // Write only
        case PPU_MASK_ADDRESS:
            Log("Illegal read of PPU Mask register", LL_WARNING);
            return ppu_state.mask; // Write only
        case PPU_STATUS_ADDRESS:
            return ppu_state.status; // Read only
        case OAM_ADDR_ADDRESS:
            Log("Illegal read of PPU OAM register", LL_WARNING);
            return ppu_state.oamaddr; // Write only
        case OAM_DATA_ADDRESS:
            return ppu_state.oamdata; // Read / Write
        case PPU_SCROLL_ADDRESS:
            Log("Illegal read of PPU Scroll register", LL_WARNING);
            return ppu_state.scroll; // Write only x2
        case PPU_ADDR_ADDRESS:
            Log("Illegal read of PPU Addr register", LL_WARNING);
            return ppu_state.ppuaddr; // Write only x2
        case PPU_DATA_ADDRESS:
            return ppu_state.ppudata; // Read / Write
        }
    }

    /*
        The APU registers are not mirrored, but the functionality between 0x4018 -> 0x401F is normally disabled
        This still results in a one-to-one map
        0x4000 -> 0x7FFF

        TODO: Savedata is stored in SRAM at 0x6000 -> 0x7FFF
   */
    else if (address < PROGRAM_ROM_ADDRESS)
    {
        if (address == OAM_DMA_ADDRESS)
        {
            return ppu_state.oamdma;
        }
        else if (address == CONTROLLER_PORT1 || address == CONTROLLER_PORT2)
        {
            Log("Controller read 1", LL_DEBUG);
            return read_controller(address);
        }

        return cpu_memory[address];
    }

    /*
        If the PRG ROM size is 16KB the address space is mirrored as such 
        0x8000 -> 0xBFFF Map
        0xC000 -> 0xFFFF Mirror to the address above

        If the PRGROM size is 32KB the address space is mapped as such
        0x8000 -> 0xFFFF Map
    */
    else if (address >= PROGRAM_ROM_ADDRESS)
    {
        // 16 KB PRG ROM size
        if (header.prg_rom_size == 1)
        {
            address = address >= PROGRAM_ROM_ADDRESS + PROGRAM_BANK_SIZE ? address - PROGRAM_BANK_SIZE : address;
        }
        // 32 KB PRG ROM size
        else if (header.prg_rom_size == 2)
        {
            // No mapping needed
        }

        return cpu_memory[address];
    }
}

void mapper0_write_memory(uint16_t address, uint8_t value)
{
    /*
        The PRG RAM can be of size 2KB, 4KB or 8KB
        in the case of 2KB, all addresses are mapped as such
        0x0000 -> 0x07FF Map
        0x0800 -> 0x0FFF Mirror
        0x1000 -> 0x17FF Mirror
        0x1800 -> 0x1FFF Mirror

        In the case of 4KB, all addresses are mapped as such
        0x0000 -> 0x0FFF Map
        0x1000 -> 0x1FFF Mirror

        In the case of 8KB, all addresses are mapped as such
        0x0000 -> 0x1FFF Map

        TODO: There seems like there only is 8KB in the iNes format
    */
    if (address < INTERNAL_RAM_BANK_SIZE * 4)
    {
        // 8KB internal RAM
        if (header.prg_ram_size == 1)
        {
            // No mirroring is needed
        }
        else
        {
            Logf("Illegal program ram size: %d", LL_WARNING, header.prg_rom_size);
        }

        cpu_memory[address] = value;
    }

    /*
        The PPU I/O registers at 0x2000 -> 0x2007 are mirrored at every 0x0008 address (8 bytes), thus
        0x2000 -> 0x2007 Map
        0x2008 -> 0x200F Mirror
        0x2010 -> 0x2017 Mirror
        ...
        0x3FF8 -> 0x3FFF Mirror 
    */
    else if (address < PPU_REGISTER_ADDRESS + PPU_REGISTER_SIZE)
    {
        address = PPU_REGISTER_ADDRESS + (address % 0x0008);
        switch (address)
        {
        case PPU_CTRL_ADDRESS:
            ppu_state.ctrl = value; // Write only
            break;
        case PPU_MASK_ADDRESS:
            ppu_state.mask = value; // Write only
            break;
        case PPU_STATUS_ADDRESS:
            ppu_state.status = value; // Read only
            Log("Illegal write to PPU Status register", LL_WARNING);
            break;
        case OAM_ADDR_ADDRESS:
            ppu_state.oamaddr = value; // Write only
            break;
        case OAM_DATA_ADDRESS:
            ppu_state.oamdata = value; // Read / Write
            break;
        case PPU_SCROLL_ADDRESS:
            ppu_state.scroll = value; // Write only x2
            break;
        case PPU_ADDR_ADDRESS:
            ppu_state.ppuaddr = value; // Write only x2
            ppu_state.ppuaddr_written = TRUE;
            break;
        case PPU_DATA_ADDRESS:
            ppu_state.ppudata = value; // Read / Write
            ppu_state.ppudata_written = TRUE;
            break;
        }
    }

    /*
        The APU registers are not mirrored, but the functionality between 0x4018 -> 0x401F is normally disabled
        This still results in a one-to-one map
        0x4000 -> 0x7FFF

        TODO: Savedata is stored in SRAM at 0x6000 -> 0x7FFF
   */
    else if (address < PROGRAM_ROM_ADDRESS)
    {
        if (address == OAM_DMA_ADDRESS)
        {
            ppu_state.oamdma = value;
            perform_oam_dma(value);
        }
        else if (address == CONTROLLER_PORT1 || address == CONTROLLER_PORT2)
        {
            Log("Controller read 3", LL_DEBUG);
            write_controller(address, value);
        }
        else
        {
            cpu_memory[address] = value;
        }
    }

    /*
        If the PRG ROM size is 16KB the address space is mirrored as such 
        0x8000 -> 0xBFFF Map
        0xC000 -> 0xFFFF Mirror to the address above

        If the PRGROM size is 32KB the address space is mapped as such
        0x8000 -> 0xFFFF Map
    */
    else if (address >= PROGRAM_ROM_ADDRESS)
    {
        // 16 KB PRG ROM size
        if (header.prg_rom_size == 1)
        {
            address = address >= PROGRAM_ROM_ADDRESS + PROGRAM_BANK_SIZE ? address - PROGRAM_BANK_SIZE : address;
        }
        // 32 KB PRG ROM size
        else if (header.prg_rom_size == 2)
        {
            // No mapping needed
        }

        cpu_memory[address] = value;
    }
}

uint8_t *mapper0_get_memory_pointer(uint16_t address)
{
    /*
        The PRG RAM can be of size 2KB, 4KB or 8KB
        in the case of 2KB, all addresses are mapped as such
        0x0000 -> 0x07FF Map
        0x0800 -> 0x0FFF Mirror
        0x1000 -> 0x17FF Mirror
        0x1800 -> 0x1FFF Mirror

        In the case of 4KB, all addresses are mapped as such
        0x0000 -> 0x0FFF Map
        0x1000 -> 0x1FFF Mirror

        In the case of 8KB, all addresses are mapped as such
        0x0000 -> 0x1FFF Map

        TODO: There seems like there only is 8KB in the iNes format
    */
    if (address < INTERNAL_RAM_BANK_SIZE * 4)
    {
        // 8KB internal RAM
        if (header.prg_ram_size == 1)
        {
            // No mirroring is needed
        }
        else
        {
            Logf("Illegal program ram size: %d", LL_WARNING, header.prg_rom_size);
        }

        return &cpu_memory[address];
    }

    /*
        The PPU I/O registers at 0x2000 -> 0x2007 are mirrored at every 0x0008 address (8 bytes), thus
        0x2000 -> 0x2007 Map
        0x2008 -> 0x200F Mirror
        0x2010 -> 0x2017 Mirror
        ...
        0x3FF8 -> 0x3FFF Mirror 
    */
    else if (address < PPU_REGISTER_ADDRESS + PPU_REGISTER_SIZE)
    {
        address = PPU_REGISTER_ADDRESS + (address % 0x0008);
        switch (address)
        {
        case PPU_CTRL_ADDRESS:
            return &ppu_state.ctrl; // Write only
        case PPU_MASK_ADDRESS:
            return &ppu_state.mask; // Write only
        case PPU_STATUS_ADDRESS:
            return &ppu_state.status; // Read only
        case OAM_ADDR_ADDRESS:
            return &ppu_state.oamaddr; // Write only
        case OAM_DATA_ADDRESS:
            return &ppu_state.oamdata; // Read / Write
        case PPU_SCROLL_ADDRESS:
            return &ppu_state.scroll; // Write only x2
        case PPU_ADDR_ADDRESS:
            return &ppu_state.ppuaddr; // Write only x2
        case PPU_DATA_ADDRESS:
            return &ppu_state.ppudata; // Read / Write
        }
    }

    /*
        The APU registers are not mirrored, but the functionality between 0x4018 -> 0x401F is normally disabled
        This still results in a one-to-one map
        0x4000 -> 0x7FFF

        TODO: Savedata is stored in SRAM at 0x6000 -> 0x7FFF
   */
    else if (address < PROGRAM_ROM_ADDRESS)
    {
        if (address == OAM_DMA_ADDRESS)
        {
            return &ppu_state.oamdma;
        }

        return &cpu_memory[address];
    }

    /*
        If the PRG ROM size is 16KB the address space is mirrored as such 
        0x8000 -> 0xBFFF Map
        0xC000 -> 0xFFFF Mirror to the address above

        If the PRGROM size is 32KB the address space is mapped as such
        0x8000 -> 0xFFFF Map
    */
    else if (address >= PROGRAM_ROM_ADDRESS)
    {
        // 16 KB PRG ROM size
        if (header.prg_rom_size == 1)
        {
            address = address >= PROGRAM_ROM_ADDRESS + PROGRAM_BANK_SIZE ? address - PROGRAM_BANK_SIZE : address;
        }
        // 32 KB PRG ROM size
        else if (header.prg_rom_size == 2)
        {
            // No mapping needed
        }

        return &cpu_memory[address];
    }
}

void mapper0_write_to_pointer(uint8_t *ptr, uint8_t value)
{
    // Assign the vlaue
    *ptr = value;

    // Handle the special cases
    if (ptr == &ppu_state.ppuaddr)
    {
        ppu_state.ppuaddr_written = TRUE;
    }
    else if (ptr == &ppu_state.ppudata)
    {
        ppu_state.ppudata_written = TRUE;
    }
    else if (ptr == &ppu_state.oamdma)
    {
        perform_oam_dma(value);
    }
    else if (ptr == &ppu_state.oamaddr)
    {
    }
    else if (ptr == &ppu_state.oamdata)
    {
    }
    else if (ptr == &cpu_memory[CONTROLLER_PORT1])
    {
        Log("Controller write 2", LL_DEBUG);
        write_controller(CONTROLLER_PORT1, value);
    }
}

uint8_t mapper0_read_pointer(uint8_t *ptr)
{
    // Handle the special cases
    if (ptr == &cpu_memory[CONTROLLER_PORT1])
    {
        Log("Controller read 4", LL_DEBUG);
        return read_controller(CONTROLLER_PORT1);
    }

    return *ptr;
}

uint8_t mapper0_ppu_read(uint16_t address)
{

    if (address >= 0x3000 && address <= 0x3EFF)
    {
        address -= 0x1000;
    }
    else if (address >= 0x3F20 && address <= 0x3FFF)
    {
        address = 0x3F00 | (address % 0x20);
    }

    return ppu_memory[address];
}

void mapper0_ppu_write(uint16_t address, uint8_t value)
{
    if (address >= 0x3000 && address <= 0x3EFF)
    {
        address -= 0x1000;
    }
    else if (address >= 0x3F20 && address <= 0x3FFF)
    {
        address = 0x3F00 | (address % 0x20);
    }

    ppu_memory[address] = value;
}

uint8_t mapper0_oam_read(uint8_t address)
{
    return oam_memory[address];
}

void mapper0_oam_write(uint8_t address, uint8_t value)
{
    oam_memory[address] = value;
}

#endif // MAPPER0_H