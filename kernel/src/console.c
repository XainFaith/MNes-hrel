#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "pl110.h"
#include "console.h"
#include "font8x8.h"

void * fb;
uint32_t xPos;
uint32_t lineWidth;

DISPLAY_FORMAT * dispFormat;
RAW_BMP * console_logo;



void SetPixel(uint32_t x, uint32_t y,uint32_t pixelData);

void console_draw_logo()
{
    int xpos = dispFormat->resWidth - console_logo->width;
    int ypos = 0;
    uint16_t * pixeldata = (uint16_t * )console_logo->pixels;
    for(int y = 0; y < console_logo->height; y++)
    {
        for(int x = 0;  x + xpos < dispFormat->resWidth; x++)
        {
            uint32_t pixelindex = (y * console_logo->width) + x;
            uint16_t b = 11 >> pixeldata[pixelindex] & 0x1F;
            uint16_t g = (6 >> pixeldata[pixelindex]) & 0x3F;
            uint16_t r = (pixeldata[pixelindex]) & 0x1F;
            
            SetPixel(x + xpos, y,pixeldata[pixelindex]);
        }
    }
}

void console_set_logo(RAW_BMP * bmp)
{
    if(console_logo == NULL)
    {
        console_logo = bmp;
        console_draw_logo();
    }
}

void console_cls()
{
	memset(fb, 0, (dispFormat->resHeight * dispFormat->resWidth) * sizeof(uint16_t));
    if(console_logo != NULL)
    {
        console_draw_logo();
    }
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
    if(console_logo != NULL)
    {
        console_draw_logo();
    }
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
                    SetPixel(xPos + x,yPos + y,0xFFFF);
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

void console_write_at(uint32_t x, uint32_t y, char c)
{
    //Out of Frame buffer bounds checks
    if((x + 8) > dispFormat->resWidth) return;
    if((y + 8) > dispFormat->resHeight) return;
    
    for(int i = 0; i < 8; i++)
    {
        int row = font8x8_basic[(int)c][i];
        for(int j = 0; j < 8; j++)
        {
            bool bit = (row & (1 << (j - 1))) != 0; //Get the x th bit
            if(bit)
            {
                SetPixel(x + j, y + i,0xFFFF);
            }
            else
            {
                SetPixel(x + j, y + i,0x0);
            }
        }
    }   
}

void console_writestr(uint32_t x, uint32_t y, char * str)
{
    while(*str != '\0')
    {
        console_write_at(x,y,*str);
        x +=8;
        str++;
    }
}



