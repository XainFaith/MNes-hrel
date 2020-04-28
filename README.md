# MNes-rhel


Metal nes, projects Aim is to create a regular Nintendo emulator to run on bare ARM systems specifically those that run an Arm1176.
For example a raspberrypi and Possibly other systems.  Currently I only have a raspberrypi for when I start doing hardware tests.

This a conversion of the project I was originally doing in C/C++ specifically for application to RHEL.

Completly written in C with the exception where ARM asm is required.

Its current state is WIP.

road map:

Physical memory managment  		-done
ported libc11		   		-done
liballoc implemented       		-done
LCD display pl110 Versatilepb varient.	-done
Graphical console text output		-done
MMCI (Reading) pl181			-done
MMCI (Writing) pl181			-defered
Master Boot Record 			-done
Fat32 file system WIP 			-inprogress
Virtual file system			-schedueled
Nes bus 				-Needs Porting
Nes 6502				-Needs Porting
Other Nes				-defered
Input GPIO / Qemu keyboard		-defered

Some of the code is in need of a clean up and a refactor but my time is limited and i prefer to work on new parts
but will some time soon need to do so.



