GCC := ~/opt/cross/bin/arm-none-eabi-gcc
AS := ~/opt/cross/bin/arm-none-eabi-as
LD := ~/opt/cross/bin/arm-none-eabi-ld

SYS_MEM :=  128 


CFLAGS := -ffreestanding  -Wall -Wextra  -I ~/opt/cross/lib/gcc/arm-none-eabi-/10.0.1/include/ -I  inc/

DEFINES :=  -D _HAVE_SIZE_T -D LACKS_UNISTD_H -D LACKS_SYS_PARAM_H -D LACKS_FCNTL_H -D LACKS_SYS_MMAN_H -D SYS_MEM=$(SYS_MEM)

ASRC := $(wildcard src/*.S) 
CSRC := $(wildcard src/*.c)
SRC := $(ASRC:%.S=%.ao) $(CSRC:%.c=%.o) 
OBJ := $(subst src, obj, $(SRC))  


obj/%.ao : src/%.S
	@echo "Assembeling " $<
	$(AS) -march=armv7-a -mcpu=cortex-a15 $< -o $@ -g

obj/%.o: src/%.c
	@echo "compiling " $<
	$(GCC) $(CFLAGS) $(DEFINES) -c $< -o $@ -g 

all : $(OBJ)
	$(LD) -T linker.ld $(OBJ) -o ../bin/kernel.bin -L ~/opt/cross/lib/gcc/arm-none-eabi/10.0.1/ -L ~/opt/cross/libc/lib/ -lgcc -lc 
	
clean:
	rm obj/*.ao
	rm obj/*.o
		