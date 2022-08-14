#include <Windows.h>
#include <fileapi.h>
#include <stdio.h>
#include "../logger.h"
#include "loader.h"
#include "cpu.h"
#include "mappers/mapper0.h"

header_t header;
uint8_t *cartrage = NULL;
mapper_t mapper;

LOAD_STATUS loadNESFile(HANDLE hfile)
{
    // Get the file size in bytes
    DWORD fileSize = GetFileSize(hfile, NULL);
    Logf("File is of size %d", LL_INFO, fileSize);

    if(cartrage == NULL)
    {
        cartrage = VirtualAlloc(NULL, fileSize, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
        Logf("Cartrage memory allocated at %p", LL_DEBUG, cartrage);
    }
    DWORD numBytesRead = 0;
    ReadFile(hfile, cartrage, fileSize, &numBytesRead, NULL);

    // Check if the file starts with "NES" for the iNES format (ID String)
    if (cartrage[0] == 'N' && cartrage[1] == 'E' && cartrage[2] == 'S' && cartrage[3] == 0x1a)
    {
        Log("File is of iNES format", LL_INFO);
    }
    else
    {
        Log("File is not of iNES format", LL_WARNING);

        return UNRECOGNIZED_FORMAT;
    }

    // Check if it is also the NES20 format
    if (cartrage[7] & 0x0c == 0x08)
    {
        Log("File is of NES20 format", LL_INFO);
        header.nes_format = NES20;
    }
    else
    {
        Log("File is of regular iNES format", LL_INFO);
        header.nes_format = iNES;
        header.prg_rom_size = cartrage[4];
        header.chr_rom_size = cartrage[5];

        // Flag 6 (mapper number is here)
        header.mirroring = (cartrage[6] & (0b1 << MIRRORING_BIT_OFFSET)) >> MIRRORING_BIT_OFFSET;
        header.persistent_mem = (cartrage[6] & (0b1 << PERSISTENT_MEM_BIT_OFFSET)) >> PERSISTENT_MEM_BIT_OFFSET;
        header.trainer = (cartrage[6] & (0b1 << TRAINER_BIT_OFFSET)) >> TRAINER_BIT_OFFSET;
        header.ignore_mirroring_control = (cartrage[6] & (0b1 << IGNORE_MIRRORING_CONTROL_BIT_OFFSET)) >> IGNORE_MIRRORING_CONTROL_BIT_OFFSET;
        header.mapper_number = (cartrage[6] >> LSB_MAPPER_NUM_NIBLE_OFFSET) | (cartrage[7] & (0b1111 << MSB_MAPPER_NUM_NIBLE_OFFSET));

        // Flag 7
        header.VS_unisystem = (cartrage[7] & (0b1 << VS_UNISYSTEM_BIT_OFFSET)) >> VS_UNISYSTEM_BIT_OFFSET;
        header.play_choice = (cartrage[7] & (0b1 << PLAYCHOICE_BIT_OFFSET)) >> PLAYCHOICE_BIT_OFFSET;

        // Flag 8s
        header.prg_ram_size = cartrage[8] ? cartrage[8] : 1; // A value of 0 is 8 KB

        // Flag 9
        header.tv_system = cartrage[9] & 0b1;
    }

    logINESHeader();

    if(header.mapper_number == 0)
    {
        uint16_t pgrMemOffset = HEADER_SIZE + (header.trainer ? TRAINER_SIZE : 0);
        uint16_t chrMemOffset = pgrMemOffset + PROGRAM_BANK_SIZE * header.prg_rom_size;

        // PRG ROM size is 16KB or 32KB
        if(header.prg_rom_size <= 2)
        {
            memcpy(cpu_memory + PROGRAM_ROM_ADDRESS, cartrage + pgrMemOffset, PROGRAM_BANK_SIZE * header.prg_rom_size);
        }
        else
        {
            Log("Illegal program rom size", LL_WARNING);
        }

        // Load char rom
        if(header.chr_rom_size == 1)
        {
            memcpy(ppu_memory, cartrage + chrMemOffset, PATTERN_TABLE_SIZE * 2);
        }
        else
        {
            Log("Illegal character rom size", LL_WARNING);
        }
        
        mapper.read_memory = &mapper0_read_memory;
        mapper.write_memory = &mapper0_write_memory;
        mapper.get_memory_pointer = &mapper0_get_memory_pointer;
        mapper.write_to_pointer = &mapper0_write_to_pointer;
        mapper.read_pointer = &mapper0_read_pointer;
        mapper.ppu_read_memory = &mapper0_ppu_read;
        mapper.ppu_write_memory = &mapper0_ppu_write;
        mapper.oam_read = &mapper0_oam_read;
        mapper.oam_write = &mapper0_oam_write;
        
        return SUCCESS;
    }
    else
    {
        Logf("Mapper not implemented: %d", LL_ERROR, header.mapper_number);
        return FAILED;
    }

    return FAILED;
}

void logINESHeader()
{
    if(header.nes_format == iNES)
    {
        Log("File format: INES", LL_INFO);
        Logf("PRG rom size: %d KB", LL_INFO, header.prg_rom_size * 16);
        Logf("CHR rom size: %d KB", LL_INFO, header.chr_rom_size * 8);
        Logf("Mapper number: %d", LL_INFO, header.mapper_number);
        Logf("Ignore mirroring: %s", LL_INFO, header.ignore_mirroring_control ? "TRUE" : "FALSE");
        Logf("Mirroring: %d", LL_INFO, header.mirroring);
        Logf("Persistent Memory: %s", LL_INFO, header.persistent_mem ? "TRUE" : "FALSE");
        Logf("Trainer: %s", LL_INFO, header.trainer ? "TRUE" : "FALSE");
        Logf("VS Unisystem: %s", LL_INFO, header.VS_unisystem ? "TRUE" : "FALSE");
        Logf("PlayChoice-10: %s", LL_INFO, header.play_choice ? "TRUE" : "FALSE");
        Logf("PRG ram size; %d KB", LL_INFO, header.prg_ram_size * 8);
        Logf("TV system: %s", LL_INFO, header.tv_system ? "PAL" : "NTSC");
    }
    else if(header.nes_format == NES20)
    {

    }
    else
    {
        Log("Unrecognized file format", LL_WARNING);
    }
}