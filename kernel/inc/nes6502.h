#ifndef _NES_6502_H
#define _NES_6502_H

#include <stdint.h>

typedef uint8_t (*nes_op)();
typedef uint8_t (*nes_addr_mode)();

typedef struct NES_REGISTERS
{
    //6502 Registers
    uint8_t A, X, Y, P, SP;
    //6502 Program Counter
    uint16_t PC;
    
    uint8_t * stackbase;
} NES_REGISTERS;

enum nes_PFlags
{
    nes_flag_C = ( 1 << 0),
    nes_flag_Z = ( 1 << 1),
    nes_flag_I = ( 1 << 2),
    nes_flag_D = ( 1 << 3),
    nes_flag_B = ( 1 << 4),
    nes_flag_U = ( 1 << 5),
    nes_flag_V = ( 1 << 6),
    nes_flag_N = ( 1 << 7)
};

//Instruction Set and Addressing Mode stuff below
typedef struct INSTRUCTION
{
    char * name;
    nes_op operation;
    nes_addr_mode addr_mode;
    uint8_t cycles;
}INSTRUCTION;


void init_nes_6502();
bool nes_getflag(enum nes_PFlags flag);
void nes_setflag(enum nes_PFlags flag, bool clear);


//nes addressing modes
uint8_t nes_IMP();
uint8_t nes_IMM();
uint8_t nes_ZP0();
uint8_t nes_ZPX();
uint8_t nes_ZPY();
uint8_t nes_REL();
uint8_t nes_ABS();
uint8_t nes_ABX();
uint8_t nes_ABY();
uint8_t nes_IND();
uint8_t nes_IZX();
uint8_t nes_IZY();

//Instruction Definitions
uint8_t nes_UDF(); //Undefined instruction catch all

uint8_t nes_ADC();	uint8_t nes_AND();	uint8_t nes_ASL();	uint8_t nes_BCC();
uint8_t nes_BCS();	uint8_t nes_BEQ();	uint8_t nes_BIT();	uint8_t nes_BMI();
uint8_t nes_BNE();	uint8_t nes_BPL();	uint8_t nes_BRK();	uint8_t nes_BVC();
uint8_t nes_BVS();	uint8_t nes_CLC();	uint8_t nes_CLD();	uint8_t nes_CLI();
uint8_t nes_CLV();	uint8_t nes_CMP();	uint8_t nes_CPX();	uint8_t nes_CPY();
uint8_t nes_DEC();	uint8_t nes_DEX();	uint8_t nes_DEY();	uint8_t nes_EOR();
uint8_t nes_INC();	uint8_t nes_INX();	uint8_t nes_INY();	uint8_t nes_JMP();
uint8_t nes_JSR();	uint8_t nes_LDA();	uint8_t nes_LDX();	uint8_t nes_LDY();
uint8_t nes_LSR();	uint8_t nes_NOP();	uint8_t nes_ORA();	uint8_t nes_PHA();
uint8_t nes_PHP();	uint8_t nes_PLA();	uint8_t nes_PLP();	uint8_t nes_ROL();
uint8_t nes_ROR();	uint8_t nes_RTI();	uint8_t nes_RTS();	uint8_t nes_SBC();
uint8_t nes_SEC();	uint8_t nes_SED();	uint8_t nes_SEI();	uint8_t nes_STA();
uint8_t nes_STX();	uint8_t nes_STY();	uint8_t nes_TAX();	uint8_t nes_TAY();
uint8_t nes_TSX();	uint8_t nes_TXA();	uint8_t nes_TXS();	uint8_t nes_TYA();


void nes_dissasemble_display();
void nes_reset();
void nes_clock();

#endif

