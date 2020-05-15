#include <string.h>
#include <stdio.h>
#include "pl110.h"



struct Registers //This is a vartion of the PL110 registers as the VersatilePB has mask irq register and the control register swapped
{
        volatile uint32_t tim0;		//0
        volatile uint32_t tim1;		//4
        volatile uint32_t tim2;		//8
		volatile uint32_t tim3;		//8
        
        volatile uint32_t upbase;	//10
		volatile uint32_t lpbase;	//10

        volatile uint16_t control;	//1c
		
		volatile uint32_t misc;
		volatile uint32_t ris;
		volatile uint32_t mis;
};

struct Registers * registers;
volatile uint16_t  * frameBuffer;
uint32_t resWidth;
uint32_t resHeight;
DISPLAY_FORMAT * displayFormat;
 
 int init_pl110(uint32_t baseAddr)
 {
	 uint32_t* clcdctrl = (uint32_t*)0x10000050;
	*clcdctrl |= 4;
	 
	 registers = (struct Registers*)baseAddr;
    /* 640x480 pixels */
	registers->tim0 = 0x3f1f3f9c;
	registers->tim1 = 0x080b61df;

    resWidth = 640;
    resHeight = 480;

    frameBuffer = (uint16_t *)malloc(  (resWidth * resHeight) * sizeof(uint16_t));

    frameBuffer = (uint16_t*)memset(frameBuffer, 0,640 * 480 * 2);

    registers->upbase = (uint32_t)frameBuffer;

    //Set 16 bit color
    registers->control = 0x1829;
		
	displayFormat = malloc(sizeof(DISPLAY_FORMAT));
	
	displayFormat->resHeight = resHeight;
	displayFormat->resWidth = resWidth;
	displayFormat->colorMode = Color16;
	
	return 0;
 }
 
 void * getframebuffer()
 {
	 return (void *)frameBuffer;
 }
 
DISPLAY_FORMAT * getdisplayformat()
{
	return displayFormat;
}
 