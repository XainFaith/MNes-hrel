
.PHONY = all kernel

bin/kernel.bin:
	make -C kernel/  all

all : bin/kernel.bin
	
run : all
	qemu-system-arm -M versatilepb -cpu arm1176 -m 128 -kernel bin/kernel.bin -s -S  -sd disk.img
	
clean:
	rm -r bin
	mkdir bin
	make -C kernel/ clean
