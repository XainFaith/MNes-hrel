#include <stdint.h>
#include "nesppu.h"

uint8_t nes_ppu_tblname[2][1024];
uint8_t nes_ppu_tblpallete[32];

uint8_t nes_cpu_ppu_read(uint16_t addr, bool readonly)
{
    uint8_t data = 0x0;
    switch(addr)
    {
        case 0x0000:    //Control register
            break;
        case 0x0001:    //Mask register
            break;
        case 0x0002:    //Status register
            break;
        case 0x0003:    //OAM Address register
            break;
        case 0x0004:    //OAM Data register
            break;
        case 0x0005:    //Scroll Register
            break;
        case 0x0006:    //PPU Address register
            break;
        case 0x0007:    //PPU Data Register
            break;
    }
    
    return data;
}

void nes_cpu_ppu_write(uint16_t addr, uint8_t data)
{
        switch(addr)
    {
        case 0x0000:    //Control register
            break;
        case 0x0001:    //Mask register
            break;
        case 0x0002:    //Status register
            break;
        case 0x0003:    //OAM Address register
            break;
        case 0x0004:    //OAM Data register
            break;
        case 0x0005:    //Scroll Register
            break;
        case 0x0006:    //PPU Address register
            break;
        case 0x0007:    //PPU Data Register
            break;
    }
}

uint8_t nes_ppu_read(uint16_t addr, bool readonly)
{
   uint8_t data = 0x0;
   data &= 0x3FFF;
   return data;
}

void nes_ppu_write(uint16_t addr, uint8_t data)
{
    addr &= 0x3FFF;
}

void clock()
{
    
}
