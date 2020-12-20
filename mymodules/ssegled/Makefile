obj-m = ssegled.o
ARCH := arm
KERNEL_DIR ?= ../../../linux
CROSS_COMPILE ?= ../../../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-

all:
	make -C $(KERNEL_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(PWD)/$(CROSS_COMPILE) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(PWD)/$(CROSS_COMPILE) M=$(PWD) clean
