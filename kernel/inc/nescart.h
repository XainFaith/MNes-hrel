#ifndef _NES_CARTRIDGE_H
#define _NES_CARTRIDGE_H

#include <stdint.h>
#include "nesmapper.h"

#define ROM_OK 0
#define BAD_ROM -2
#define ROM_NOT_FOUND -1
#define ROM_MISSING_MAPPER - 2

typedef struct NES_CARTIDGE
{
    uint8_t * prg_mem;
    uint32_t prg_size;
    
    uint8_t * chr_mem;
    uint32_t chr_size;
    
    uint8_t mapper_id;
    uint8_t prg_banks;
    uint8_t chr_banks;
    NES_MAPPER * mapper;
}NES_CARTIDGE;

typedef struct INES_HEADER
{
 char identstr[4];
 uint8_t prg_rom_chunks;
 uint8_t chr_rom_chunks;
 uint8_t mapper1;
 uint8_t mapper2;
 uint8_t prg_ram_size;
 uint8_t tv_system1;
 uint8_t tv_system2;
 char unused[4];
} INES_HEADER;

extern NES_CARTIDGE * nes_loaded_cart;

int nes_load_cart(char * filepath);

#endif
