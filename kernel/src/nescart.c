#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "nescart.h"
#include "nesmapper.h"

NES_CARTIDGE * nes_loaded_cart;

int nes_load_cart(char * filepath)
{
    if(nes_loaded_cart != NULL)
    {
        free(nes_loaded_cart->prg_mem);
        free(nes_loaded_cart->chr_mem);
        free(nes_loaded_cart);
    }
    
    NES_CARTIDGE * nes_cart = NULL;
    
    FILE * cartfile = fopen(filepath, "r");
    if(cartfile == NULL)
    {
        return ROM_NOT_FOUND;   
    }
    
    struct stat filestat;
    int statres = fstat((int)cartfile,&filestat); 
    
    if(statres != 0)
    {
        fclose(cartfile);
        return BAD_ROM;
    }
    
    char * cartbuffer = (char*)malloc(filestat.st_size);
    fread(cartbuffer, 1, filestat.st_size, cartfile);
    
    INES_HEADER * cart_header = (INES_HEADER*)cartbuffer;
    cartbuffer += 16;
    
    //Is nes header
    if(strncmp(cart_header->identstr, "NES", 3) != 0x0)
    {
        return BAD_ROM;
    }
    
    uint32_t mapperid = 0x0;
    
    if(strncmp(cart_header->unused, 0x0, 4) != 0x0)
    {
    
    if((cart_header->mapper1 & 0x0C) == 0x08)
    {
        if(cart_header->tv_system1 > filestat.st_size)
        {
            //NES 2.0 cart
        }
    }
    }
    else
    {
            //INES HEADER
            mapperid = (cart_header->mapper2 & 0xF0) | (cart_header->mapper1 >> 4);
            nes_cart = (NES_CARTIDGE*)malloc(sizeof(NES_CARTIDGE));
            //Check for presence of trainer on cart
            if((cart_header->mapper1 & 0x4) == 0x4)
            {
                //TODO load trainer data to where it shoulb go 0x7000 for cpu
                cartbuffer += 512;
            }

            nes_cart->mapper_id = mapperid;
            
            int prg_rom_chunks = cart_header->prg_rom_chunks;
            nes_cart->prg_mem = (uint8_t*)malloc(prg_rom_chunks * 16384);
            memcpy(nes_cart->prg_mem, cartbuffer,prg_rom_chunks * 16384);
            nes_cart->prg_size = prg_rom_chunks * 16384;
            nes_cart->prg_banks = prg_rom_chunks;
            
            cartbuffer += prg_rom_chunks * 16384;
            int chr_rom_chunks = cart_header->chr_rom_chunks;
            nes_cart->chr_mem = (uint8_t*)malloc(chr_rom_chunks * 8192);
            memcpy(nes_cart->chr_mem, cartbuffer, chr_rom_chunks * 8192);
            nes_cart->chr_size = chr_rom_chunks * 8192;
            nes_cart->chr_banks = chr_rom_chunks;

            nes_cart->mapper = nes_get_mapper(mapperid);

            
            nes_loaded_cart = nes_cart;
            
            free(cartbuffer);
            fclose(cartfile);
    }
    
    if(nes_cart->mapper == NULL)
    {
        free(nes_loaded_cart->prg_mem);
        free(nes_loaded_cart->chr_mem);
        free(nes_loaded_cart);
        return ROM_MISSING_MAPPER;
    }
    
    return ROM_OK;
}
