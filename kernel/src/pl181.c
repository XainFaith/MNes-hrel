#include <stdint-gcc.h>
#include <stdbool.h>
#include <stdio.h>
#include "pl181.h"
#include "block_device.h"

//Bit wise shift operators for clarity
#define EXE (1 << 10)
#define RESP (1 << 6)
#define LRESP (RESP & ( 1 << 7 ))
#define HCS (1 << 30)

struct SD_CARD
{
	uint32_t rca;
	bool ishc;
};

struct SD_CARD * mmci_sd_card;

struct PL181_REGISTERS
{
	uint32_t power;
	uint32_t clock;
	uint32_t arg;
	uint32_t cmd;
	uint32_t respCmd;
	uint32_t response[4];
	uint32_t dataTimer;
	uint32_t dataLength;
	uint32_t dataCtrl;
	uint32_t dataCnt;
	uint32_t status;
	uint32_t clear;
	uint32_t intMask[2];
	uint32_t intSelect;
	uint32_t fifoCnt;
};

struct PL181_FIFO
{
	uint32_t data[15];
};

struct PL181_REGISTERS * mmci_registers;
struct PL181_FIFO * mmci_dataFIFO;


int init_pl181(uint32_t baseaddr)
{
	mmci_registers = (volatile struct PL181_REGISTERS*)baseaddr;
    mmci_dataFIFO = (volatile struct PL181_FIFO*)(baseaddr + 0x080);
   
	//TODO Figure out how to properly initalize the pl181 on real hardware since qemu does a half job emulating it

    mmci_registers->clock = 0x1C6;
    mmci_registers->power = 0x86;
}

void mmci_send_cmd(volatile uint32_t arg, uint32_t param)
{
	mmci_clear_flags();
	
	mmci_registers->arg = param;
	
    //Wait until the cmd line is not active
    while((mmci_registers->status & (1 << 11) != (1 << 11)))
    {}

    mmci_registers->cmd = arg;

    while(mmci_registers->status & ( 1 << 11)){}

    if((arg & RESP) == (RESP))
    {
        //If the command requested a response wait for a response
        while((mmci_registers->status & 0x40) != (1 << 6) || mmci_registers->status & ( 1 << 11))
        {
            if(mmci_registers->status & (1 << 2))
            {
                return;
            }
        }
    }
    else 
    {
        while((mmci_registers->status & (1 << 7)) != (1 << 7))
        {
        }
    }    
}

bool mmci_send_app_cmd(uint32_t arg, uint32_t param)
{
    uint32_t count = 0;
    while(count != 5)
    {
		if(mmci_sd_card == NULL)
		{
			mmci_send_cmd(55 | EXE | RESP, 0);
		}
		else
		{
				mmci_send_cmd(55, mmci_sd_card->rca);	
		}
		
        if(mmci_registers->response[0] & ( 1 << 5))
        {
            mmci_send_cmd(arg,param);
            return true;
        }
        count++;
    }

    printf("Failed to send app command \n");

    return false;
}

void mmci_set_arg(uint32_t arg)
{
    mmci_registers->arg = arg;
}

int mmci_select_card(uint32_t rca, bool hc)
{
	mmci_send_cmd(7 | EXE | RESP, rca);
	return 0;
}

int mmci_read_block(void * buffer, uint32_t addr)
{
	uint32_t * readBuffer = (uint32_t*)buffer;
    uint32_t bytesRead = 0;
 
    mmci_registers->dataLength = 512; //Set length of bytes to transfer
    mmci_registers->dataCtrl = (0x3); //Enable dpsm and set direction from card to host
 
    if(mmci_sd_card->ishc)
    {
        mmci_send_cmd(17 | EXE | RESP, addr);
    }
    else
    {
        mmci_send_cmd(17 | EXE | RESP, addr * 512);
    }
   
    if((mmci_registers->status & 1 << 3) == true)
    {
        return bytesRead; //return number of bytes read before error
    }
 
    while((mmci_registers->status & (1 << 21))) //read status bit for if the fifo still has data avaliable, seems more reliable then the fifo counter
    {
        *readBuffer = mmci_dataFIFO->data[0];
        readBuffer++;
        bytesRead +=4;
    }
    return bytesRead;
}

int mmci_write_block(void * buffer, uint32_t addr)
{
    return 0;
}

void mmci_clear_resp_registers()
{
    mmci_registers->response[0] = 0;
    mmci_registers->response[1] = 0;
    mmci_registers->response[2] = 0;
    mmci_registers->response[3] = 0;
}

void mmci_clear_flags()
{
    mmci_registers->clear = 0x5FF;
}

struct BLOCK_DEVICE * get_block_device()
{

    //Send cmd 0 to set all cards into idle state
    mmci_send_cmd(0 | EXE, 0x0);
 
    //Voltage Check
    mmci_send_cmd(8 | EXE | RESP, 0x1AA);
    if((mmci_registers->response[0] | 0x1AA) != 0x1AA)
    {
        return false;
    }
 
    //send acmd 41
    if(mmci_send_app_cmd(41 | EXE | RESP, 1 << 30 | 0x1AA) == false)
    {
        return false;
    }
 
    bool isHC = false;
 
    if(mmci_registers->response[0] & 1 << 30)
    {
        isHC = true;
    }
 
   
    while((mmci_registers->response[0] | (1 << 31)) == false) //Wait until busy bit is cleared and the card has initalized
    {}
 
    //Send All send CID
    mmci_send_cmd(2 | EXE | LRESP,0x0); //Not terribly relavent for my usage but does return manufacture info etc
 
    //Send cmd 3 for to get card RCA
    mmci_send_cmd(3 | EXE | RESP,0x0);
 
    //Get the card RCA from the response it is the top 16 bits of the 32 bit response
    uint32_t rca = (mmci_registers->response[0] & 0xFFFF0000 );
 
	mmci_sd_card = (struct SD_CARD*)malloc(sizeof(struct SD_CARD));
 
    mmci_sd_card->ishc = isHC;
    mmci_sd_card->rca = rca;
 
    //Select the card
    mmci_select_card(mmci_sd_card->rca, mmci_sd_card->ishc);
 
    //Set data length of a block
    mmci_send_cmd(16 | EXE | RESP, 512 );
    if(mmci_registers->response[0] == 0x900) //TODO Figure out how to check for a busy signal on the data line for a response of r1b
    {
        printf("card is selected and in the transfer state \n");
    }
    else
    {
        printf("failed to select card!: \n");
    }
	
	struct BLOCK_DEVICE * card = (struct BLOCK_DEVICE *)malloc(sizeof(struct BLOCK_DEVICE));
	card->present = true;
	card->read = &mmci_read_block;
	card->write = &mmci_write_block;
	
	return card;
}