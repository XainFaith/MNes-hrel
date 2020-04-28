ENTRY(_start)

SECTIONS 
{
	. = 0x10000;
	
	.boot   : {obj/boot.ao(text.boot)}
	.text   : {*(.text)}
	.rodata : {*(.rodata)}
	.data   : {*(.data)}
	
	__bss_start = .;
	
	.bss    : {*(.bss COMMON)}
	
	__bss_end = .;
	
	__stack_end = .;
	. += 0x10000;
	__stack_base = .;
	
	.  = ALIGN(4096);
	__kernel_end = .;
}