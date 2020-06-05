#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bmp.h"


RAW_BMP * bmp_to16bitarray(char * buffer)
{
    if(buffer == NULL)
    {
        return NULL;
    }
    
    BMP_TYPE_HEADER * bmp_header = (BMP_TYPE_HEADER*)buffer;
    if(strncmp((char*)bmp_header->type, "BM", 2) != 0x0) //Is the data an actual bmp
    {
        return NULL;
    }
    
    //Get the DIB header
    BMP_DIB_HEADER * dib_header = (BMP_DIB_HEADER*)(buffer + 14);
    if(dib_header->header_size == 0x0) return NULL;
    if(dib_header->bits_per_pixel != 16) return NULL; //Only supporting 16 bits per pixel formats
    
    printf("img Width: %lu , img height: %lu \n", dib_header->img_width, dib_header->img_height);
    printf("Total colors: %lu \n", dib_header->total_colors);
    
    if(dib_header->bits_per_pixel != 16)
    {
        return NULL;
    }

    //Check for compression 0x0 is no compress and a a 555 color schema where 0x3 is 565
    if(dib_header->compression == 0x0 || dib_header->compression == 0x3)
    {
        RAW_BMP * rawbmp = (RAW_BMP*)malloc(sizeof(RAW_BMP));
        rawbmp->pixels = (char*)malloc(dib_header->img_size);

        for(int i =dib_header->img_height; i > 0; i--)
        {
            uint16_t * bufptr = (uint16_t*)rawbmp->pixels;
            bufptr += (dib_header->img_height * dib_header->img_width);
            bufptr -= (i * dib_header->img_width);
            
            uint16_t * srcptr = (uint16_t*)(buffer + bmp_header->parrayoffset);
            srcptr += (dib_header->img_height * dib_header->img_width);
            srcptr -= (i * dib_header->img_width);
            memcpy(bufptr, srcptr, dib_header->img_width * 2);            
        }
        
        rawbmp->height = dib_header->img_height;
        rawbmp->width = dib_header->img_width;
        return rawbmp;
    }
    
    return NULL;
}
