#ifndef _CONSOLE_H
#define _CONSOLE_H

#include "bmp.h"

int init_console();
void console_write(char c);
void console_writestr(uint32_t x, uint32_t y, char * str);
void console_cls();

void console_set_logo(RAW_BMP * bmp);

#endif 
