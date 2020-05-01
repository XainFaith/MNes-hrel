#include <stdbool.h>
#include <stdint-gcc.h>
#include <stdio.h>
#include <string.h>
#include "pl110.h"
#include "console.h"
#include "font8x8.h"

void * fb;
uint32_t xPos;
uint32_t lineWidth;

DISPLAY_FORMAT * dispFormat;

void console_cls()
{
	memset(fb, 0, (dispFormat->resHeight * dispFormat->resWidth) * sizeof(uint16_t));
}

int init_console()
{
	dispFormat = getdisplayformat();
	fb = getframebuffer();
	 lineWidth = dispFormat->resWidth / 8; 
	 console_cls();
	 return 0;
}

void SetPixel(uint32_t x, uint32_t y,uint32_t pixelData)
{
	((uint16_t*)fb)[(y * dispFormat->resWidth) + x] = (uint16_t)pixelData;
}

void Scroll()
{
	uint16_t * fb16 = (uint16_t*)fb;
    memcpy(fb16, fb16 + (sizeof(uint16_t) * dispFormat->resWidth * 8), sizeof(uint16_t) * dispFormat->resWidth * dispFormat->resHeight);
	memset(fb16 + (sizeof(uint16_t) * dispFormat->resWidth * (dispFormat->resHeight-8)),0,sizeof(uint16_t) * dispFormat->resWidth * 8);
}

void console_write(char c)
{
	 if(c == '\n')
    {
        xPos = 0;
        Scroll();
        return;
    }

    //Get pixel coords for current glyph to be drawn
    uint32_t yPos = dispFormat->resHeight - 8;
    //Font is 8 x 8
    int y =0;
    for(y = 0; y < 8; y++) //y is row
    {
        int row = font8x8_basic[(int)c][y]; //Get the data corresponding to the row
        for(int x = 0; x < 8; x++) //x is bit of row
        {
                bool bit = (row & (1 << (x - 1))) != 0; //Get the x th bit
                if(bit)
                {
                    SetPixel(xPos + x,yPos + y,0xFFFE);
                }
                else
                {
                    SetPixel(xPos + x,yPos + y,0x0);
                }
        }
    }

    xPos += 8;
    if(xPos >= (dispFormat->resWidth - 8))
    {
        xPos = 0;
        Scroll();
    }
}




