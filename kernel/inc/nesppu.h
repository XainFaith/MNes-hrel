#ifndef _NES_PPU_H
#define _NES_PPU_H

#include <stdint.h>
#include <stdbool.h>
#include "nescart.h"

uint8_t nes_cpu_ppu_read(uint16_t addr, bool readonly);
void nes_cpu_ppu_write(uint16_t addr, uint8_t data);

uint8_t nes_ppu_read(uint16_t addr, bool readonly);
void nes_ppu_write(uint16_t addr, uint8_t data);

void clock();

#endif
