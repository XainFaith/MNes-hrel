#include <stdint.h>
#include <stdlib.h>
#include "nesbus.h"
#include "nesppu.h"
#include "nescart.h"
#include "nesmapper.h"

//Pointer to Nes Ram
uint8_t * nes_cpu_ram;

void init_nes_bus()
{
    if(nes_cpu_ram == NULL)
    {
        nes_cpu_ram = (uint8_t*)malloc(2048);
    }
}

void nes_cpu_write(uint16_t addr, uint8_t data)
{
    //is in cpu address range
    if(addr <= 0x1FFF)
    {
        nes_cpu_ram[addr & 0x07FF] = data;
        return;
    }
    
    //is ppu address range
    if(addr >= 0x2000 && addr <= 0x3FFF)
    {
        nes_cpu_ppu_write(addr & 0x0007, data);
        return;
    }
    
    nes_loaded_cart->mapper->write(addr, data);
}

uint8_t nes_cpu_read(uint16_t addr, bool readOnly)
{
    uint8_t data = 0;
    //is in cpu address range
    if(addr <= 0x1FFF)
    {
        return nes_cpu_ram[addr & 0x07FF];
    }
    
    //is ppu address range
    if(addr >= 0x2000 && addr <= 0x3FFF)
    {
        return nes_cpu_ppu_read(addr & 0x0007, readOnly);
    }

    return nes_loaded_cart->mapper->read(addr);
}

void nes_bus_reset()
{
    
}

void nes_bus_clock()
{
    
}
