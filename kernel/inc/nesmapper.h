#ifndef _NES_MAPPER_H
#define _NES_MAPPER_H

#include <stdint.h>


typedef uint8_t (*nes_mapper_read)(uint16_t addr);
typedef void (*nes_mapper_write)(uint16_t addr, uint8_t data);

typedef struct
{
    int id;
    nes_mapper_read read;
    nes_mapper_write write;
    
} NES_MAPPER;

NES_MAPPER * nes_get_mapper(int mapperid);

#endif
