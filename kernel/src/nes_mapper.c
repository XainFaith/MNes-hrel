#include <stdint.h>
#include <stdlib.h>

#include "nescart.h"
#include "nesmapper.h"


uint8_t nes_mapper_zero_read(uint16_t addr)
{
    if(nes_loaded_cart->prg_banks <= 1)
    {
        if(addr >= 0x8000 && addr <= 0xBFFF)
        {
            return nes_loaded_cart->prg_mem[addr - 0x8000];
        }
        else if(addr >= 0xC000 && addr <= 0xFFFF)
        {
            return nes_loaded_cart->prg_mem[addr - 0xC000];
        }
    }
    
    return 0;
}

void nes_mapper_zero_write(uint16_t addr, uint8_t data)
{
    if(addr && data)
    {}
}


NES_MAPPER * nes_get_mapper(int mapperid)
{
    
    NES_MAPPER * mapper = (NES_MAPPER*)malloc(sizeof(NES_MAPPER));
    switch(mapperid)
    {
        case 0:
            mapper->id = 0;
            mapper->read = &nes_mapper_zero_read;
            mapper->write = &nes_mapper_zero_write;
        return mapper;    
        
        default:
            free(mapper);
    }
    
    return NULL;
}

