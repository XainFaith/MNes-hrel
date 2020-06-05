#ifndef BMP_H
#define BMP_H

#include <stdint.h>

typedef struct __attribute__((packed))
{
    unsigned char type[2];
    uint32_t size;
    uint32_t reserved;
    uint32_t parrayoffset;
} BMP_TYPE_HEADER;

typedef struct __attribute__((packed))
{
    uint32_t header_size;
    int32_t img_width;
    int32_t img_height;
    uint16_t img_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t img_size;
    int32_t xpixels_per_meter;
    int32_t ypixels_per_meter;
    uint32_t total_colors;
    uint32_t important_colors;
} BMP_DIB_HEADER;

enum BMP_BPP
{
    MONO = 1,
    FOUR_BIT_PAL = 4,
    EIGHT_BIT_PAL = 8,
    SIX_TEEN_BIT_RGB = 16,
    TWENTY_FOUR_BIT_RGB = 24
};

typedef struct RAW_BMP
{
    enum BMP_BPP bpp;
    int width;
    int height;
    char * pixels;
}RAW_BMP;

RAW_BMP * bmp_to16bitarray(char * buffer);

#endif
