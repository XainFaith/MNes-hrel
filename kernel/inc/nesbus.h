#ifndef _NES_BUS_H
#define _NES_BUS_H

#include <stdint.h>
#include <stdbool.h>

#include "nescart.h"

void init_nes_bus();

void nes_cpu_write(uint16_t addr, uint8_t data);
uint8_t nes_cpu_read(uint16_t addr, bool readOnly);

void nes_bus_reset();
void nes_bus_clock();


#endif
