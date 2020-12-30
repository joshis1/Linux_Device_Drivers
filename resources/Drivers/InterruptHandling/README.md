How to compile this module?
============

	$make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ../../KernelSource_4_19v/ M=$(pwd) modules


How to edit this README.md?
============

	$remarkable README.md

How to clean up  this module?
============

	$make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C ../../KernelSource_4_19v/ M=$(pwd) clean
	
	
How to see the module information?
============

	$/sbin/modinfo main.ko
	filename:       /home/shreyas/ldd/Linux_Device_Drivers/resources/Drivers/HelloWorld/main.ko
	board:          Zedboard - AVNET
	description:    Hello World Program
	author:         Shreyas Joshi
	license:        GPL
	depends:        
	name:           main
	vermagic:       4.19.0-xilinx SMP preempt mod_unload modversions ARMv7 p2v8 


How to see the module sections?
============

	$arm-linux-gnueabihf-objdump main.ko  -x

	main.ko:     file format elf32-littlearm
	main.ko
	architecture: arm, flags 0x00000011:
	HAS_RELOC, HAS_SYMS
	start address 0x00000000
	private flags = 5000000: [Version5 EABI]
	
	Sections:
	Idx Name          Size      VMA       LMA       File off  Algn
  	0 .note.gnu.build-id 00000024  00000000  00000000  00000034  2**2
                  CONTENTS, ALLOC, LOAD, READONLY, DATA
  	1 .text         00000000  00000000  00000000  00000058  2**0
                  CONTENTS, ALLOC, LOAD, READONLY, CODE
  	2 .init.text    00000018  00000000  00000000  00000058  2**2
    	 CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
  	3 .exit.text    0000000c  00000000  00000000  00000070  2**2
                  CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
                  
  References - 
  
  [https://stackoverflow.com/questions/60923890/how-to-build-linux-kernel-module-using-yocto-sdk](https://stackoverflow.com/questions/60923890/how-to-build-linux-kernel-module-using-yocto-sdk) 
  
  
  