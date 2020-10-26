obj-m := pcd_n.o
ARCH=arm
CROSS_COMPILE=arm-linux-gnueabihf-
KERN_DIR=../../KernelSource_4_19v/

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) clean

help:
	make ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(KERN_DIR) M=$(PWD) help

host:
	make -C /lib/modules/$(shell uname -r)/build  M=$(PWD) modules

	
	
	
	
