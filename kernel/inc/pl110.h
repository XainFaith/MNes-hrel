#ifndef _PL110_H
#define _PL110_H

#include <stdint.h>

enum ColorMode
{
    Pallete,
    Color16,
    Color32
};

typedef struct 
{
	uint32_t resWidth;
	uint32_t resHeight;
	enum ColorMode colorMode;
} DISPLAY_FORMAT;


int init_pl110(uint32_t baseAddr);
void * getframebuffer();
DISPLAY_FORMAT * getdisplayformat();

#endif
