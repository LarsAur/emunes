#ifndef LOADER_H

#define LOADER_H

#include <windows.h>
#include <stdint.h>

#define MIRRORING_BIT_OFFSET 0
#define PERSISTENT_MEM_BIT_OFFSET 1
#define TRAINER_BIT_OFFSET 2
#define IGNORE_MIRRORING_CONTROL_BIT_OFFSET 3
#define LSB_MAPPER_NUM_NIBLE_OFFSET 4

#define VS_UNISYSTEM_BIT_OFFSET 0
#define PLAYCHOICE_BIT_OFFSET 1
#define MSB_MAPPER_NUM_NIBLE_OFFSET 4

#define HEADER_SIZE 16
#define TRAINER_SIZE 512

typedef enum LOAD_STATUS
{
    SUCCESS,
    FAILED,
    UNRECOGNIZED_FORMAT,
} LOAD_STATUS;

typedef enum FILE_FORMAT
{
    iNES,
    NES20,
} FILE_FORMAT;

typedef enum MIRRORING
{
    HORIZONTAL = 0,
    VERTICAL = 1,
} MIRRORING;

typedef enum TV_SYSTEM
{
    NTSC = 0,
    PAL = 1,
} TV_SYSTEM;

typedef struct header_t
{
    uint64_t constant;
    uint16_t prg_rom_size; // 16KB units
    uint16_t chr_rom_size; // 8KB units
    uint16_t mapper_number;
    uint8_t sub_mapper_number;

    // Flag 6 NES 1
    MIRRORING mirroring;
    BOOL persistent_mem;
    BOOL trainer;
    BOOL ignore_mirroring_control;

    // Flag 7 NES 1
    BOOL VS_unisystem;
    BOOL play_choice;
    FILE_FORMAT nes_format;

    // Flag 8 NES 1
    uint8_t prg_ram_size;    // Volatile (shift count in the case of NES20, 64 << sc)
    uint8_t prg_nv_ram_size; // Non-volaile (Shift count in the case of NES20, 64 << sc). If sc is 0 there is none

    uint8_t tv_system;
} header_t;

typedef struct mapper_t
{
    uint8_t *(*get_memory_pointer)(uint16_t);                  // Returning a pointer to the byte at the loaction in ram
    void (*write_to_pointer)(uint8_t *ptr, uint8_t value);
    uint8_t (*read_pointer)(uint8_t *ptr);
    uint8_t (*read_memory)(uint16_t address);                  // Reading a byte from ram
    void (*write_memory)(uint16_t address, uint8_t value);     // Writing a byte to ram
    uint8_t (*ppu_read_memory)(uint16_t address);          
    void (*ppu_write_memory)(uint16_t address, uint8_t value); 
    uint8_t (*oam_read)(uint8_t address);
    void (*oam_write)(uint8_t address, uint8_t value);
} mapper_t;

extern header_t header;
extern uint8_t *cartrage;
extern mapper_t mapper;

LOAD_STATUS loadNESFile(HANDLE hfile);
void logINESHeader();

#endif