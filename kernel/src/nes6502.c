#include <stddef.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include "nes6502.h"
#include "nesbus.h"

#include "console.h"

NES_REGISTERS * nes_registers;
INSTRUCTION * nes_instructions;

uint16_t nes_temp;
uint8_t  nes_fetched;       // Represents the working input value to the ALU
uint16_t nes_addr_abs;      // All used memory addresses end up in here
uint16_t nes_addr_rel;      // Represents absolute address following a branch
uint8_t  nes_opcode;        // Is the instruction byte
uint8_t  nes_cycles;	    // Counts how many nes_cycles the instruction has remaining
uint32_t nes_clock_count;	// A global accumulation of the number of clocks

uint8_t * nes_stack_base;

void nes_add_ins(char * name, nes_op op, nes_addr_mode addrmode, int cycles)
{
    nes_instructions[nes_temp].name = name;
    nes_instructions[nes_temp].cycles = cycles;
    nes_instructions[nes_temp].operation = op;
    nes_instructions[nes_temp].addr_mode = addrmode;
    nes_temp++;
}

void init_nes_6502()
{ 
 if(nes_registers == NULL)
 {
    nes_registers = (NES_REGISTERS*)malloc(sizeof(NES_REGISTERS));
 }
 
 if(nes_instructions == NULL)
 {
    nes_temp = 0;
    nes_instructions = (INSTRUCTION*)malloc(sizeof(INSTRUCTION) * 0xFF);   
    
    nes_add_ins( "BRK", &nes_BRK, &nes_IMM, 7 );
    nes_add_ins( "ORA", &nes_ORA, &nes_IZX, 6 ); 
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 3 );
    nes_add_ins( "ORA", &nes_ORA, &nes_ZP0, 3 );
    nes_add_ins( "ASL", &nes_ASL, &nes_ZP0, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 5 );
    nes_add_ins( "PHP", &nes_PHP, &nes_IMP, 3 );
    nes_add_ins( "ORA", &nes_ORA, &nes_IMM, 2 );
    nes_add_ins( "ASL", &nes_ASL, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "ORA", &nes_ORA, &nes_ABS, 4 );
    nes_add_ins( "ASL", &nes_ASL, &nes_ABS, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "BPL", &nes_BPL, &nes_REL, 2 );
    nes_add_ins( "ORA", &nes_ORA, &nes_IZY, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "ORA", &nes_ORA, &nes_ZPX, 4 );
    nes_add_ins( "ASL", &nes_ASL, &nes_ZPX, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "CLC", &nes_CLC, &nes_IMP, 2 );
    nes_add_ins( "ORA", &nes_ORA, &nes_ABY, 4 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "ORA", &nes_ORA, &nes_ABX, 4 );
    nes_add_ins( "ASL", &nes_ASL, &nes_ABX, 7 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "JSR", &nes_JSR, &nes_ABS, 6 );
    nes_add_ins( "AND", &nes_AND, &nes_IZX, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "BIT", &nes_BIT, &nes_ZP0, 3 );
    nes_add_ins( "AND", &nes_AND, &nes_ZP0, 3 );
    nes_add_ins( "ROL", &nes_ROL, &nes_ZP0, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 5 );
    nes_add_ins( "PLP", &nes_PLP, &nes_IMP, 4 );
    nes_add_ins( "AND", &nes_AND, &nes_IMM, 2 );
    nes_add_ins( "ROL", &nes_ROL, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "BIT", &nes_BIT, &nes_ABS, 4 );
    nes_add_ins( "AND", &nes_AND, &nes_ABS, 4 );
    nes_add_ins( "ROL", &nes_ROL, &nes_ABS, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "BMI", &nes_BMI, &nes_REL, 2 );
    nes_add_ins( "AND", &nes_AND, &nes_IZY, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "AND", &nes_AND, &nes_ZPX, 4 );
    nes_add_ins( "ROL", &nes_ROL, &nes_ZPX, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "SEC", &nes_SEC, &nes_IMP, 2 );
    nes_add_ins( "AND", &nes_AND, &nes_ABY, 4 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "AND", &nes_AND, &nes_ABX, 4 );
    nes_add_ins( "ROL", &nes_ROL, &nes_ABX, 7 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "RTI", &nes_RTI, &nes_IMP, 6 );
    nes_add_ins( "EOR", &nes_EOR, &nes_IZX, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 3 );
    nes_add_ins( "EOR", &nes_EOR, &nes_ZP0, 3 );
    nes_add_ins( "LSR", &nes_LSR, &nes_ZP0, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 5 );
    nes_add_ins( "PHA", &nes_PHA, &nes_IMP, 3 );
    nes_add_ins( "EOR", &nes_EOR, &nes_IMM, 2 );
    nes_add_ins( "LSR", &nes_LSR, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "JMP", &nes_JMP, &nes_ABS, 3 );
    nes_add_ins( "EOR", &nes_EOR, &nes_ABS, 4 );
    nes_add_ins( "LSR", &nes_LSR, &nes_ABS, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "BVC", &nes_BVC, &nes_REL, 2 );
    nes_add_ins( "EOR", &nes_EOR, &nes_IZY, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "EOR", &nes_EOR, &nes_ZPX, 4 );
    nes_add_ins( "LSR", &nes_LSR, &nes_ZPX, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "CLI", &nes_CLI, &nes_IMP, 2 );
    nes_add_ins( "EOR", &nes_EOR, &nes_ABY, 4 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "EOR", &nes_EOR, &nes_ABX, 4 );
    nes_add_ins( "LSR", &nes_LSR, &nes_ABX, 7 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "RTS", &nes_RTS, &nes_IMP, 6 );
    nes_add_ins( "ADC", &nes_ADC, &nes_IZX, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 3 );
    nes_add_ins( "ADC", &nes_ADC, &nes_ZP0, 3 );
    nes_add_ins( "ROR", &nes_ROR, &nes_ZP0, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 5 );
    nes_add_ins( "PLA", &nes_PLA, &nes_IMP, 4 );
    nes_add_ins( "ADC", &nes_ADC, &nes_IMM, 2 );
    nes_add_ins( "ROR", &nes_ROR, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "JMP", &nes_JMP, &nes_IND, 5 );
    nes_add_ins( "ADC", &nes_ADC, &nes_ABS, 4 );
    nes_add_ins( "ROR", &nes_ROR, &nes_ABS, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "BVS", &nes_BVS, &nes_REL, 2 );
    nes_add_ins( "ADC", &nes_ADC, &nes_IZY, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "ADC", &nes_ADC, &nes_ZPX, 4 );
    nes_add_ins( "ROR", &nes_ROR, &nes_ZPX, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "SEI", &nes_SEI, &nes_IMP, 2 );
    nes_add_ins( "ADC", &nes_ADC, &nes_ABY, 4 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "ADC", &nes_ADC, &nes_ABX, 4 );
    nes_add_ins( "ROR", &nes_ROR, &nes_ABX, 7 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "STA", &nes_STA, &nes_IZX, 6 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "STY", &nes_STY, &nes_ZP0, 3 );
    nes_add_ins( "STA", &nes_STA, &nes_ZP0, 3 );
    nes_add_ins( "STX", &nes_STX, &nes_ZP0, 3 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 3 );
    nes_add_ins( "DEY", &nes_DEY, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "TXA", &nes_TXA, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "STY", &nes_STY, &nes_ABS, 4 );
    nes_add_ins( "STA", &nes_STA, &nes_ABS, 4 );
    nes_add_ins( "STX", &nes_STX, &nes_ABS, 4 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 4 );
    nes_add_ins( "BCC", &nes_BCC, &nes_REL, 2 );
    nes_add_ins( "STA", &nes_STA, &nes_IZY, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "STY", &nes_STY, &nes_ZPX, 4 );
    nes_add_ins( "STA", &nes_STA, &nes_ZPX, 4 );
    nes_add_ins( "STX", &nes_STX, &nes_ZPY, 4 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 4 );
    nes_add_ins( "TYA", &nes_TYA, &nes_IMP, 2 );
    nes_add_ins( "STA", &nes_STA, &nes_ABY, 5 );
    nes_add_ins( "TXS", &nes_TXS, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 5 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 5 );
    nes_add_ins( "STA", &nes_STA, &nes_ABX, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 5 );
    nes_add_ins( "LDY", &nes_LDY, &nes_IMM, 2 );
    nes_add_ins( "LDA", &nes_LDA, &nes_IZX, 6 );
    nes_add_ins( "LDX", &nes_LDX, &nes_IMM, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "LDY", &nes_LDY, &nes_ZP0, 3 );
    nes_add_ins( "LDA", &nes_LDA, &nes_ZP0, 3 );
    nes_add_ins( "LDX", &nes_LDX, &nes_ZP0, 3 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 3 );
    nes_add_ins( "TAY", &nes_TAY, &nes_IMP, 2 );
    nes_add_ins( "LDA", &nes_LDA, &nes_IMM, 2 );
    nes_add_ins( "TAX", &nes_TAX, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "LDY", &nes_LDY, &nes_ABS, 4 );
    nes_add_ins( "LDA", &nes_LDA, &nes_ABS, 4 );
    nes_add_ins( "LDX", &nes_LDX, &nes_ABS, 4 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 4 );
    nes_add_ins( "BCS", &nes_BCS, &nes_REL, 2 );
    nes_add_ins( "LDA", &nes_LDA, &nes_IZY, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 5 );
    nes_add_ins( "LDY", &nes_LDY, &nes_ZPX, 4 );
    nes_add_ins( "LDA", &nes_LDA, &nes_ZPX, 4 );
    nes_add_ins( "LDX", &nes_LDX, &nes_ZPY, 4 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 4 );
    nes_add_ins( "CLV", &nes_CLV, &nes_IMP, 2 );
    nes_add_ins( "LDA", &nes_LDA, &nes_ABY, 4 );
    nes_add_ins( "TSX", &nes_TSX, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 4 );
    nes_add_ins( "LDY", &nes_LDY, &nes_ABX, 4 );
    nes_add_ins( "LDA", &nes_LDA, &nes_ABX, 4 );
    nes_add_ins( "LDX", &nes_LDX, &nes_ABY, 4 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 4 );
    nes_add_ins( "CPY", &nes_CPY, &nes_IMM, 2 );
    nes_add_ins( "CMP", &nes_CMP, &nes_IZX, 6 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "CPY", &nes_CPY, &nes_ZP0, 3 );
    nes_add_ins( "CMP", &nes_CMP, &nes_ZP0, 3 );
    nes_add_ins( "DEC", &nes_DEC, &nes_ZP0, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 5 );
    nes_add_ins( "INY", &nes_INY, &nes_IMP, 2 );
    nes_add_ins( "CMP", &nes_CMP, &nes_IMM, 2 );
    nes_add_ins( "DEX", &nes_DEX, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "CPY", &nes_CPY, &nes_ABS, 4 );
    nes_add_ins( "CMP", &nes_CMP, &nes_ABS, 4 );
    nes_add_ins( "DEC", &nes_DEC, &nes_ABS, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "BNE", &nes_BNE, &nes_REL, 2 );
    nes_add_ins( "CMP", &nes_CMP, &nes_IZY, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "CMP", &nes_CMP, &nes_ZPX, 4 );
    nes_add_ins( "DEC", &nes_DEC, &nes_ZPX, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "CLD", &nes_CLD, &nes_IMP, 2 );
    nes_add_ins( "CMP", &nes_CMP, &nes_ABY, 4 );
    nes_add_ins( "NOP", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "CMP", &nes_CMP, &nes_ABX, 4 );
    nes_add_ins( "DEC", &nes_DEC, &nes_ABX, 7 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "CPX", &nes_CPX, &nes_IMM, 2 );
    nes_add_ins( "SBC", &nes_SBC, &nes_IZX, 6 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "CPX", &nes_CPX, &nes_ZP0, 3 );
    nes_add_ins( "SBC", &nes_SBC, &nes_ZP0, 3 );
    nes_add_ins( "INC", &nes_INC, &nes_ZP0, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 5 );
    nes_add_ins( "INX", &nes_INX, &nes_IMP, 2 );
    nes_add_ins( "SBC", &nes_SBC, &nes_IMM, 2 );
    nes_add_ins( "NOP", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_SBC, &nes_IMP, 2 );
    nes_add_ins( "CPX", &nes_CPX, &nes_ABS, 4 );
    nes_add_ins( "SBC", &nes_SBC, &nes_ABS, 4 );
    nes_add_ins( "INC", &nes_INC, &nes_ABS, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "BEQ", &nes_BEQ, &nes_REL, 2 );
    nes_add_ins( "SBC", &nes_SBC, &nes_IZY, 5 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 8 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "SBC", &nes_SBC, &nes_ZPX, 4 );
    nes_add_ins( "INC", &nes_INC, &nes_ZPX, 6 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 6 );
    nes_add_ins( "SED", &nes_SED, &nes_IMP, 2 );
    nes_add_ins( "SBC", &nes_SBC, &nes_ABY, 4 );
    nes_add_ins( "NOP", &nes_NOP, &nes_IMP, 2 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
    nes_add_ins( "UDF", &nes_NOP, &nes_IMP, 4 );
    nes_add_ins( "SBC", &nes_SBC, &nes_ABX, 4 );
    nes_add_ins( "INC", &nes_INC, &nes_ABX, 7 );
    nes_add_ins( "UDF", &nes_UDF, &nes_IMP, 7 );
 }
 
 
}

void nes_clock()
{

    if(nes_cycles == 0)
    {
        //Read the next instruction
        nes_opcode = nes_cpu_read(nes_registers->PC, false);

        // Always set the unused status flag bit to 1
		nes_setflag(nes_flag_U, true);
		
		// Increment program counter, we nes_read the nes_opcode byte
		nes_registers->PC++;

		// Get Starting number of nes_cycles
		nes_cycles = nes_instructions[nes_opcode].cycles;

		// Perform fetch of intermmediate data using the
		// required addressing mode
		uint8_t additional_cycle1 = (nes_instructions[nes_opcode].addr_mode)();

		// Perform operation
		uint8_t additional_cycle2 = (nes_instructions[nes_opcode].operation)();

		// The addressmode and nes_opcode may have altered the number
		// of nes_cycles this instruction requires before its completed
		nes_cycles += (additional_cycle1 & additional_cycle2);

		// Always set the unused status flag bit to 1
		nes_setflag(nes_flag_U, true);

    }


	// Decrement the number of nes_cycles remaining for this instruction
	nes_cycles--;
}

//Nes Cpu Reset
void nes_reset()
{
    // Get address to set program counter to
	nes_addr_abs = 0xFFFC;
	uint16_t lo = nes_cpu_read(nes_addr_abs + 0, false);
	uint16_t hi = nes_cpu_read(nes_addr_abs + 1, false);

	// Set it
	nes_registers->PC = (hi << 8) | lo;

	// Reset internal registers
	nes_registers->A = 0;
	nes_registers->X = 0;
	nes_registers->Y = 0;
	nes_registers->SP = 0xFD;
	nes_registers->P = 0x00 | nes_flag_U;

	// Clear internal helper variables
	nes_addr_rel = 0x0000;
	nes_addr_abs = 0x0000;
	nes_fetched = 0x00;

	// Reset takes time
	nes_cycles = 8;
}

//Interrupt Request
void nes_irq()
{
    //If irq bit is not set meaning irq is not disabled
    if(nes_getflag(nes_flag_I) == 0)
    {
        // Push the program counter to the stack. It's 16-bits dont
		// forget so that takes two pushes
		nes_cpu_write(((uint32_t)nes_stack_base) + nes_registers->SP, (nes_registers->PC >> 8) & 0x00FF);
		nes_registers->SP--;
		nes_cpu_write(((uint32_t)nes_stack_base) + nes_registers->SP, nes_registers->PC & 0x00FF);
		nes_registers->SP--;

		// Then Push the status register to the stack
		nes_setflag(nes_flag_B, 0);
		nes_setflag(nes_flag_U, 1);
		nes_setflag(nes_flag_I, 1);
		nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, nes_registers->P);
		nes_registers->SP--;

		// Read new program counter location from fixed address
		nes_addr_abs = 0xFFFE;
		uint16_t lo = nes_cpu_read(nes_addr_abs + 0, false);
		uint16_t hi = nes_cpu_read(nes_addr_abs + 1, false);
		nes_registers->PC = (hi << 8) | lo;

		// IRQs take time
		nes_cycles = 7;
    }
}

//Non Maskable IRQ
void nes_nmirq()
{
    nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, (nes_registers->PC >> 8) & 0x00FF);
	nes_registers->SP--;
	nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, nes_registers->PC & 0x00FF);
	nes_registers->SP--;

	nes_setflag(nes_flag_B, 0);
	nes_setflag(nes_flag_U, 1);
	nes_setflag(nes_flag_I, 1);
	nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, nes_registers->P);
	nes_registers->SP--;

	nes_addr_abs = 0xFFFA;
	uint16_t lo = nes_cpu_read(nes_addr_abs + 0, false);
	uint16_t hi = nes_cpu_read(nes_addr_abs + 1, false);
	nes_registers->PC = (hi << 8) | lo;

	nes_cycles = 8;
}

bool nes_ins_complete()
{
    return nes_cycles == 0;
}

void nes_setflag(enum nes_PFlags flag, bool clear)
{
    if(clear)
    {
        nes_registers->P |= flag;
    }
    else
    {
        nes_registers->P &= flag;
    }
    
}

bool nes_getflag(enum nes_PFlags flag)
{
    uint8_t flagVal = nes_registers->P & flag;
    if(flagVal & 0xFF)
    {
        return true;
    }

    return false;;
}

uint8_t nes_fetch()
{
    if(!(nes_instructions[nes_opcode].addr_mode == &nes_IMP))
    {
        nes_fetched = nes_cpu_read(nes_addr_abs, false);
    }
    
    return nes_fetched;
}


//Addresing Mode Implemntations

//Addressing Mode Implied
uint8_t nes_IMP()
{
    nes_fetched = nes_registers->A;
    return 0;
}

//Addressing Mode Immediate
//This addressing mode expects the next byte to be a value, so we set the nes_read address to the point where the next byte is in the program
uint8_t nes_IMM()
{
    nes_addr_abs = nes_registers->PC++;
    return 0;
}

//Addressing Mode Zero Page
//Zero page addressing is used to save space for program bytes as such it only uses the fist Bytes of an address range
uint8_t nes_ZP0()
{
    //Read the bytes at the current PC
    nes_addr_abs = nes_cpu_read(nes_registers->PC, false);
    //Increment the program counter
    nes_registers->PC++;
    //Mask off the bits needed
    nes_addr_abs &= 0x00FF;
    return 0;
}

//Addressing Mode Zero Page with offset X
uint8_t nes_ZPX()
{
    nes_addr_abs = nes_cpu_read(nes_registers->PC + nes_registers->X, false);
    nes_registers->PC++;
    nes_addr_abs &= 0xFF;
    return 0;
}

//Addressing Mode Zero Page with offset Y
uint8_t nes_ZPY()
{
    nes_addr_abs = nes_cpu_read(nes_registers->PC + nes_registers->Y, false);
    nes_registers->PC++;
    nes_addr_abs &= 0xFF;
    return 0;
}

//Addressing Mode Relative
//Used only for branching instructions, 
uint8_t nes_REL()
{
    nes_addr_rel = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;

    if(nes_addr_rel & 0x80)
    {
        nes_addr_rel |= 0xFF00; //Set signed bit
    }

    return 0;
}

//Addressing mode Absolute, uses a full 16 bits for the address
uint8_t nes_ABS()
{
    uint16_t loaddr = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;
    uint16_t hiaddr = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;

    nes_addr_abs = ((hiaddr << 8) | loaddr);

    return 0;
}

//Addressing Mode Absolute plus X offset
uint8_t nes_ABX()
{

    uint16_t loaddr = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;
    uint16_t hiaddr = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;

    nes_addr_abs = ((hiaddr << 8) | loaddr);
    nes_addr_abs += nes_registers->X;
    
    if ((nes_addr_abs & 0xFF00) != (hiaddr << 8))
		return 1; //Additional clock cycle if the the page boundary is cross
	else
		return 0;	
}

//Addressing Mode Absolute plus Y offset
uint8_t nes_ABY()
{

    uint16_t loaddr = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;
    uint16_t hiaddr = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;

    nes_addr_abs = ((hiaddr << 8) | loaddr);
    nes_addr_abs += nes_registers->Y;
    
    if ((nes_addr_abs & 0xFF00) != (hiaddr << 8))
		return 1; //Additional clock cycle if the the page boundary is cross
	else
		return 0;	
}

//Addressing Mode Indirect
uint8_t nes_IND()
{
    uint16_t lo = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;
    uint16_t hi = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;

    uint16_t ptr = (hi << 8) | lo;

    if (lo == 0x00FF) //Simulates page boundary hardware bug
	{
		nes_addr_abs = (nes_cpu_read(ptr & 0xFF00, false) << 8) | nes_cpu_read(ptr + 0, false);
	}
	else // Behave normally
	{
		nes_addr_abs = (nes_cpu_read(ptr + 1, false) << 8) | nes_cpu_read(ptr + 0, false);
	}

    return 0;
}

uint8_t nes_IZX()
{
    uint16_t val = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;

    uint16_t lo = nes_cpu_read((uint16_t)(val + (uint16_t)nes_registers->X) & 0x00FF, false);
    uint16_t hi = nes_cpu_read((uint16_t)(val + (uint16_t)nes_registers->X + 1) * 0x00FF, false);

    nes_addr_abs = (hi << 8) | lo;

    return 0;
}

//Addressing Mode Inderct X
uint8_t nes_IZY()
{
    uint16_t val = nes_cpu_read(nes_registers->PC, false);
    nes_registers->PC++;

    uint16_t lo = nes_cpu_read(val & 0x00FF, false);
    uint16_t hi = nes_cpu_read((val + 1) & 0x00FF, false);

    nes_addr_abs = (hi << 8) | lo;
    nes_addr_abs += nes_registers->Y;

    if((nes_addr_abs & 0xFF00) != (hi << 8))
    {
        return 1; //Add an extra cycle for crossing a page boundary
    }
    else
    {
        return 0;
    }
    
}

//Instruction Implmentations

//Break Instruction, Soft IRQ
uint8_t nes_BRK()
{
    nes_registers->PC++;
    nes_setflag(nes_flag_I, true);
    //Push the current PC onto the nes stack, this is done with 2 nes_writes because the PC is 16 bits wide
    nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, ((nes_registers->PC >> 8) & 0x00FF));
    nes_registers->SP--;
    nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, nes_registers->PC & 0x00FF);
    nes_registers->SP--;

    //Set the break Flag
    nes_setflag(nes_flag_B, true);
    //Write the status registers onto the stack
    nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, nes_registers->P);
    nes_registers->SP--;

    //Clear the break flag
    nes_setflag(nes_flag_B, false);

    //Set the PC to the jump address located at 0xFFFE and 0xFFFF
    nes_registers->PC = (uint16_t)nes_cpu_read(0xFFFE, false) | ((uint16_t)nes_cpu_read(0xFFFF, false) << 8);
    
    //Return zero as no additional cpu nes_cycles are required for the operation
    return 0;
}

//Add with carry bit
uint8_t nes_ADC()
{

    nes_fetch();

    uint16_t val = (uint16_t)nes_registers->A + (uint16_t)nes_fetched + (uint16_t)nes_getflag(nes_flag_C);
    
    nes_setflag(nes_flag_C, (val > 255)); //Update Carry Flag
    nes_setflag(nes_flag_Z, (val = 0x00)); //Update Zero flag

    //Update over flow flag V = ~(A^M) & (A^R) Thanks to One Lone Coder for this one
    nes_setflag(nes_flag_V, (~(uint16_t)nes_registers->A ^ ((uint16_t)nes_fetched & ((uint16_t)nes_registers->A ^ val))) & 0x0080);

    nes_setflag(nes_flag_N, (val & 0x80)); //Update Negative Flag

    nes_registers->A = val & 0x00FF;

    return 1; //TODO Figure out when this needs to have an extra cycle and when it does not
}

//Subtrace with borrow bit
uint8_t nes_SBC()
{

    nes_fetch();

    //Get the fetched data and invert the value with an XOR operation
    uint16_t val = ((uint16_t)nes_fetched) ^ 0x00FF;
    uint16_t temp = (uint16_t)nes_registers->A + val + (uint16_t)nes_getflag(nes_flag_C);
    nes_setflag(nes_flag_C, temp & 0xFF00);
    nes_setflag(nes_flag_Z, (temp & 0x00FF) == 0x0);
    nes_setflag(nes_flag_V, (temp ^ (uint16_t)nes_registers->A) & (temp ^ val) & 0x0080);
    nes_setflag(nes_flag_N, temp & 0x80);
    nes_registers->A = temp * 0x0FF;

    return 1;   //TODO Figure out when this does and does not need an additional cycle
}

//Bitwise an Operation
uint8_t nes_AND()
{
    nes_fetch();
    nes_registers->A = nes_registers->A & nes_fetched;
    nes_setflag(nes_flag_Z, nes_registers->A == 0x00);
    nes_setflag(nes_flag_N, nes_registers->A & 0x80);

    //TODO Figure out how to determine exactly how many nes_cycles are and are not needed since not all AND instructions require additional nes_cycles
    return 1;
}

//Bitewise Arthimitc shift left
uint8_t nes_ASL()
{
    nes_fetch();

    uint16_t val = (uint16_t)nes_fetched << 1;
    nes_setflag(nes_flag_Z, (val & 0xFF) == 0x0 );
    nes_setflag(nes_flag_N, val & 0x80);
    nes_setflag(nes_flag_C, (val & 0xFF00) > 0);

    if(nes_instructions[nes_opcode].addr_mode == &nes_IMP)
    {
        nes_registers->A = nes_temp & 0xFF;
    }
    else
    {
        nes_cpu_write(nes_addr_abs, val & 0xFF);
    }
    
    return 0;
}


//Branch on Carry Clear
uint8_t nes_BCC()
{
    if(nes_getflag(nes_flag_C) == 0x0)
    {
        nes_cycles++;
        nes_addr_abs = nes_registers->PC + nes_addr_rel;

        //If the address crosses a page boundary add an extra cycle
        if((nes_addr_abs & 0xFF00) != (nes_registers->PC & 0xFF00))
        {
            nes_cycles++;
        }

        nes_registers->PC = nes_addr_abs;
    }

    return 0;
}

//Branch on Carry SET
uint8_t nes_BCS()
{
    if(nes_getflag(nes_flag_C))
    {
        nes_cycles++;
        nes_addr_abs = nes_registers->PC + nes_addr_rel;

        //If the address crosses a page boundary add an extra cycle
        if((nes_addr_abs & 0xFF00) != (nes_registers->PC & 0xFF00))
        {
            nes_cycles++;
        }

        nes_registers->PC = nes_addr_abs;
    }

    return 0;
}

//Branch if Equal
uint8_t nes_BEQ()
{
    if(nes_getflag(nes_flag_Z))
    {
       nes_cycles++;
        nes_addr_abs = nes_registers->PC + nes_addr_rel;

        //If the address crosses a page boundary add an extra cycle
        if((nes_addr_abs & 0xFF00) != (nes_registers->PC & 0xFF00))
        {
            nes_cycles++;
        }
        nes_registers->PC = nes_addr_abs;
    }

    return 0;
}

//Branch if Negative
uint8_t nes_BMI()
{
    if(nes_getflag(nes_flag_N))
    {
       nes_cycles++;
        nes_addr_abs = nes_registers->PC + nes_addr_rel;

        //If the address crosses a page boundary add an extra cycle
        if((nes_addr_abs & 0xFF00) != (nes_registers->PC & 0xFF00))
        {
            nes_cycles++;
        }
        nes_registers->PC = nes_addr_abs;
    }

    return 0;
}

//Branch if NOT equal
uint8_t nes_BNE()
{
    if(nes_getflag(nes_flag_Z) == 0)
    {
       nes_cycles++;
        nes_addr_abs = nes_registers->PC + nes_addr_rel;

        //If the address crosses a page boundary add an extra cycle
        if((nes_addr_abs & 0xFF00) != (nes_registers->PC & 0xFF00))
        {
            nes_cycles++;
        }
        nes_registers->PC = nes_addr_abs;
    }

    return 0;
}

//Branch on Positive
uint8_t nes_BPL()
{
    if(nes_getflag(nes_flag_N) == 0)
    {
     nes_cycles++;
        nes_addr_abs = nes_registers->PC + nes_addr_rel;

        //If the address crosses a page boundary add an extra cycle
        if((nes_addr_abs & 0xFF00) != (nes_registers->PC & 0xFF00))
        {
            nes_cycles++;
        }
        nes_registers->PC = nes_addr_abs;
    }

    return 0;
}

//Branch on overflow clear
uint8_t nes_BVC()
{
    if(nes_getflag(nes_flag_V) == 0)
    {
        nes_cycles++;
        nes_addr_abs = nes_registers->PC + nes_addr_rel;

        //If the address crosses a page boundary add an extra cycle
        if((nes_addr_abs & 0xFF00) != (nes_registers->PC & 0xFF00))
        {
            nes_cycles++;
        }
        nes_registers->PC = nes_addr_abs;
    }

    return 0;
}

//Branch on overflow set
uint8_t nes_BVS()
{
    if(nes_getflag(nes_flag_V))
    {
        nes_cycles++;
        nes_addr_abs = nes_registers->PC + nes_addr_rel;

        //If the address crosses a page boundary add an extra cycle
        if((nes_addr_abs & 0xFF00) != (nes_registers->PC & 0xFF00))
        {
            nes_cycles++;
        }
        nes_registers->PC = nes_addr_abs;
    }

    return 0;
}

//Clear Carry Flag
uint8_t nes_CLC()
{
    nes_setflag(nes_flag_C, false);
    return 0;
}

//Clear Decimal Flag
uint8_t nes_CLD()
{
    nes_setflag(nes_flag_D, false);
    return 0;
}

//Clear Irq Disable Bit
uint8_t nes_CLI()
{
    nes_setflag(nes_flag_I, false);
    return 0;
}

//Clear overflow Flag
uint8_t nes_CLV()
{
    nes_setflag(nes_flag_V, false);
    return 0;
}

//Compare Accumulator with Memory
uint8_t nes_CMP()
{
    nes_fetch();

    uint16_t val = (uint16_t)nes_registers->A - (uint16_t)nes_fetched;
    nes_setflag(nes_flag_C, nes_registers->A >= nes_fetched);
	nes_setflag(nes_flag_Z, (val & 0xFF) == 0x0);
	nes_setflag(nes_flag_N, val & 0x80);

	return 1;
}

//Compare X Index with Memory
uint8_t nes_CPX()
{
    nes_fetch();

    uint16_t val = (uint16_t)nes_registers->X - (uint16_t)nes_fetched;
    nes_setflag(nes_flag_C, nes_registers->X >= nes_fetched);
	nes_setflag(nes_flag_Z, (val & 0xFF) == 0x0);
	nes_setflag(nes_flag_N, val & 0x80);

	return 1;
}

//Compare Y Index with Memory
uint8_t nes_CPY()
{
    nes_fetch();

    uint16_t val = (uint16_t)nes_registers->Y - (uint16_t)nes_fetched;
    nes_setflag(nes_flag_C, nes_registers->Y >= nes_fetched);
	nes_setflag(nes_flag_Z, (val & 0xFF) == 0x0);
	nes_setflag(nes_flag_N, val & 0x80);

	return 1;
}

//Decrement Memory
uint8_t nes_DEC()
{
    nes_fetch();

    uint16_t val = (uint16_t)nes_fetched - 1;
    nes_cpu_write(nes_addr_abs, val & 0x00FF);
    nes_setflag(nes_flag_Z, (val & 0x00FF) == 0x0);
    nes_setflag(nes_flag_N, val & 0x80);

    return 0;
}

//Decrement X
uint8_t nes_DEX()
{
    nes_registers->X--;
    nes_setflag(nes_flag_N, nes_registers->X & 0x80);
    nes_setflag(nes_flag_Z, (nes_registers->X & 0xFF) == 0x0);

    return 0;
}

//Decrement Y
uint8_t nes_DEY()
{
    nes_registers->Y--;
    nes_setflag(nes_flag_N, nes_registers->Y & 0x80);
    nes_setflag(nes_flag_Z, (nes_registers->Y & 0xFF) == 0x0);

    return 0;
}

//Bitwise Exclusive Or of memory with Accumulator
uint8_t nes_EOR()
{
    nes_fetch();

    uint16_t val = nes_fetched ^ nes_registers->A;
    nes_setflag(nes_flag_Z, val == 0x0);
    nes_setflag(nes_flag_N, val & 0x80);

    return 1;
}

//Increment Memory by one
uint8_t nes_INC()
{
    nes_fetch();
    uint16_t val = nes_fetched + 1;
	nes_cpu_write(nes_addr_abs, val & 0xFF);
	nes_setflag(nes_flag_Z, (val & 0xFF) == 0x0);
	nes_setflag(nes_flag_N, val & 0x80);

	return 0;
}

//Increment X by one
uint8_t nes_INX()
{
    nes_fetch();
    nes_registers->X++;
	nes_setflag(nes_flag_Z, (nes_registers->X & 0xFF) == 0x0);
	nes_setflag(nes_flag_N, nes_registers->X & 0x80);

	return 0;
}

//Increment Y by one
uint8_t nes_INY()
{
    nes_fetch();
    nes_registers->Y++;
	nes_setflag(nes_flag_Z, (nes_registers->Y & 0xFF) == 0x0);
	nes_setflag(nes_flag_N, nes_registers->Y & 0x80);

	return 0;
}

//Jump to Address
uint8_t nes_JMP()
{
    nes_registers->PC = nes_addr_abs;
    return 0;
}

//Jump to sub routine
uint8_t nes_JSR()
{
    nes_registers->PC--;

    //Save the return address first the hi half
    nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, (nes_registers->PC >> 8) &  0xFF);
    nes_registers->SP--;
    nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, nes_registers->PC & 0xFF);
    nes_registers->SP--;

    nes_registers->PC = nes_addr_abs;
    return 0;
}

//Load to Accumulator
uint8_t nes_LDA()
{
    nes_fetch();
    nes_registers->A = nes_fetched;
    nes_setflag(nes_flag_Z, nes_registers->A == 0x0);
    nes_setflag(nes_flag_N, nes_registers->A & 0x80);
    return 1;
}	

//Load to X Register
uint8_t nes_LDX()
{
    nes_fetch();
    nes_registers->X = nes_fetched;
    nes_setflag(nes_flag_Z, nes_registers->X == 0x0);
    nes_setflag(nes_flag_N, nes_registers->X & 0x80);
    return 1;
}	

//Load to Y Register
uint8_t nes_LDY()
{
    nes_fetch();
    nes_registers->Y = nes_fetched;
    nes_setflag(nes_flag_Z, nes_registers->Y == 0x0);
    nes_setflag(nes_flag_N, nes_registers->Y & 0x80);
    return 1;
}

//Bit shift Right
uint8_t nes_LSR()
{
    nes_fetch();
	nes_setflag(nes_flag_C, nes_fetched & 0x0001);
	uint16_t temp = nes_fetched >> 1;	
	nes_setflag(nes_flag_Z, (temp & 0x00FF) == 0x0000);
	nes_setflag(nes_flag_N, temp & 0x0080);
	if (nes_instructions[nes_opcode].addr_mode == &nes_IMP)
    {
		nes_registers->A = temp & 0x00FF;
    }
	else
    {
		nes_cpu_write(nes_addr_abs, temp & 0x00FF);
    }

	return 0;
}	

//No operation Ins = Pulled from One Lone Coder source
uint8_t nes_NOP()
{
// Sadly not all NOPs are equal, Ive added a few here
	// based on https://wiki.nesdev.com/w/index.php/CPU_unofficial_nes_opcodes
	// and will add more based on game compatibility, and ultimately
	// I'd like to cover all illegal nes_opcodes too
	switch (nes_opcode) {
	case 0x1C:
	case 0x3C:
	case 0x5C:
	case 0x7C:
	case 0xDC:
	case 0xFC:
		return 1;
		break;
	}
	return 0;
}	

//Bitwise OR of Accumulator with value
uint8_t nes_ORA()
{
    nes_fetch();
    nes_registers->A = nes_registers->A | nes_fetched;
    nes_setflag(nes_flag_Z, nes_registers->A == 0x0);
    nes_setflag(nes_flag_N, nes_registers->A & 0x80);
    return 1;
}	

//Push Accumulator to stack
uint8_t nes_PHA()
{
    nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, nes_registers->A);
    nes_registers->SP--;
    return 0;
}

//Push CPU Status onto Stack
uint8_t nes_PHP()
{
    nes_cpu_write(((uint32_t)nes_stack_base)  + nes_registers->SP, nes_registers->P);
    nes_registers->SP--;
    nes_setflag(nes_flag_B, 0);
    nes_setflag(nes_flag_U, 0);
    return 0;
}	

//Pull Accumulator from Stack
uint8_t nes_PLA()
{
    nes_registers->SP++;
    nes_registers->A = nes_cpu_read(((uint32_t)nes_stack_base) + nes_registers->SP, false);
    nes_setflag(nes_flag_Z, nes_registers->A == 0x0);
    nes_setflag(nes_flag_N, nes_registers->A & 0x80);
    return 0;
}

//Pull Cpu Status from Stack
uint8_t nes_PLP()
{
    nes_registers->SP++;
    nes_registers->P = nes_cpu_read(((uint32_t)nes_stack_base) + nes_registers->SP, false);
    nes_setflag(nes_flag_U, 1);
    return 0;
}	

//Rotate one bit left
uint8_t nes_ROL()
{
	nes_fetch();
	uint16_t temp = (uint16_t)(nes_fetched << 1) | nes_getflag(nes_flag_C);
	nes_setflag(nes_flag_C, temp & 0xFF00);
	nes_setflag(nes_flag_Z, (temp & 0x00FF) == 0x0000);
	nes_setflag(nes_flag_N, temp & 0x0080);
	if (nes_instructions[nes_opcode].addr_mode == &nes_IMP)
    {
		nes_registers->A = temp & 0x00FF;
    }
	else
    {
		nes_cpu_write(nes_addr_abs, temp & 0x00FF);
    }

	return 0;
}

//Rotate one bit right
uint8_t nes_ROR()
{
    nes_fetch();
	uint16_t temp = (uint16_t)(nes_getflag(nes_flag_C) << 7) | (nes_fetched >> 1);
	nes_setflag(nes_flag_C, nes_fetched & 0x01);
	nes_setflag(nes_flag_Z, (temp & 0x00FF) == 0x00);
	nes_setflag(nes_flag_N, temp & 0x0080);
	if (nes_instructions[nes_opcode].addr_mode == &nes_IMP)
    {
		nes_registers->A = temp & 0x00FF;
    }
	else
    {
		nes_cpu_write(nes_addr_abs, temp & 0x00FF);
    }

	return 0;
}	

//Return from IRQ
uint8_t nes_RTI()
{
    nes_registers->SP++;
    nes_registers->P = nes_cpu_read(((uint32_t)nes_stack_base) + nes_registers->SP, false);

    nes_setflag(nes_flag_B, ~nes_flag_B);
    nes_setflag(nes_flag_U, ~nes_flag_U);

    nes_registers->SP++;
    nes_registers->PC = (uint16_t)nes_cpu_read(((uint32_t)nes_stack_base) + nes_registers->SP, false);
    nes_registers->SP++;
    nes_registers->PC |= (uint16_t)nes_cpu_read(((uint32_t)nes_stack_base) + nes_registers->SP, false) << 8;
    return 0;
}	

//Return from Sub Routine
//pull PC, PC+1 -> PC 
uint8_t nes_RTS()
{
    nes_registers->SP++;
    nes_registers->PC = nes_cpu_read(((uint32_t)nes_stack_base) + nes_registers->SP, false);
    nes_registers->SP++;
    nes_registers->PC |= nes_cpu_read(((uint32_t)nes_stack_base) + nes_registers->SP, false) << 8;
    nes_registers->PC++;

    return 0;
}	

//Set Carry Flag
uint8_t nes_SEC()
{
    nes_setflag(nes_flag_C, 1);
    return 0;
}	

//Set Decimal Flag
uint8_t nes_SED()
{
    nes_setflag(nes_flag_D, 1);
    return 0;
}	

//Set IRQ Disable Flag
uint8_t nes_SEI()
{
    nes_setflag(nes_flag_I,1);
    return 0;
}	

//Store Accumulator in memory
uint8_t nes_STA()
{
    nes_cpu_write(nes_addr_abs, nes_registers->A);
    return 0;
}

//Store X in memory
uint8_t nes_STX()
{
    nes_cpu_write(nes_addr_abs, nes_registers->X);
    return 0;
}	

//Store Y in memory
uint8_t nes_STY()
{
    nes_cpu_write(nes_addr_abs, nes_registers->Y);
    return 0;
}	

//Transfer Accumulator to X index
uint8_t nes_TAX()
{
    nes_registers->X = nes_registers->A;
    nes_setflag(nes_flag_N, nes_registers->X == 0x0);
    nes_setflag(nes_flag_Z, nes_registers->X & 0x80);

    return 0;
}	

//Transfer Accumulator to Y index
uint8_t nes_TAY()
{
    nes_registers->Y = nes_registers->A;
    nes_setflag(nes_flag_Z, nes_registers->Y == 0x0);
    nes_setflag(nes_flag_N, nes_registers->Y & 0x80);

    return 0;
}

//Transfer Stack Pointer to X
uint8_t nes_TSX()
{
    nes_registers->X = nes_registers->SP;
    nes_setflag(nes_flag_Z, nes_registers->X == 0x0);
    nes_setflag(nes_flag_N, nes_registers->X & 0x80);

    return 0;
}	

//Transfer X register to accumulator
uint8_t nes_TXA()
{
    nes_registers->A = nes_registers->X;
    nes_setflag(nes_flag_Z, nes_registers->A == 0x0);
    nes_setflag(nes_flag_N, nes_registers->A & 0x80);

    return 0;
}	

//Transfer X Register to stack register
uint8_t nes_TXS()
{
    nes_registers->SP = nes_registers->X;
    return 0;
}	

//Transfer Y Register to Accumulator
uint8_t nes_TYA()
{
    nes_registers->A = nes_registers->Y;
    nes_setflag(nes_flag_Z, nes_registers->A == 0x0);
    nes_setflag(nes_flag_N, nes_registers->A & 0x80);

    return 0;
}

//Bit Tst 
uint8_t nes_BIT()
{
    nes_fetch();

    uint8_t val = nes_registers->A & nes_fetched;
    nes_setflag(nes_flag_Z, (val == 0));
    nes_setflag(nes_flag_N, (nes_fetched & ( 1 << 7)));
    nes_setflag(nes_flag_V, (nes_fetched & ( 1 << 6)));

    return 0;
}

uint8_t nes_UDF()
{
    //Undefined instruction
    printf("NES: undefined instruction was executed! \n");
    return 0;
}

void nes_dissasemble_display()
{
    uint16_t addr = nes_registers->PC;
    
    int x = 256 + 32;
    int y = 64;
    
    for(int i =0; i < 10; i++)
    {
        uint8_t opcode = nes_cpu_read(addr, true);
        addr++;
        
        char * disstr = (char*)malloc(256);
        *disstr = '$';
        disstr++;
        
        char *opname = nes_instructions[opcode].name;
        strcpy(disstr, opname);
        disstr += strlen(opname) + 1;

        if(nes_instructions[opcode].addr_mode == &nes_IMP)
        {
            strcpy(disstr, "{IMP}");
            console_writestr( x, y, disstr);
            free(disstr);
        }
        else if(nes_instructions[opcode].addr_mode == &nes_IMM)
        {
            uint8_t value = nes_cpu_read(addr, true);
            strcpy(disstr, "{IMM} ");
            disstr+=6;
            
        }
        else if(nes_instructions[opcode].addr_mode == &nes_ZP0)
        {
            
        }
        else if(nes_instructions[opcode].addr_mode == &nes_ZPX)
        {
            
        }
        else if(nes_instructions[opcode].addr_mode == &nes_ZPY)
        {
            
        }
        else if(nes_instructions[opcode].addr_mode == &nes_REL)
        {
            
        }
        else if(nes_instructions[opcode].addr_mode == &nes_ABS)
        {
            
        }
        else if(nes_instructions[opcode].addr_mode == &nes_ABX)
        {
            
        }
        else if(nes_instructions[opcode].addr_mode == &nes_ABY)
        {
            
        }
        else if(nes_instructions[opcode].addr_mode == &nes_IND)
        {
            
        }
        else if(nes_instructions[opcode].addr_mode == &nes_INX)
        {
            
        }
        else if(nes_instructions[opcode].addr_mode == &nes_INY)
        {
            
        }
        else
        {
            //Something went wrong here
            console_writestr(x,y,"Uknown Opcode or Addressing Mode!");
            free(disstr);
        }
        
        y += 12;
    }
    
}

