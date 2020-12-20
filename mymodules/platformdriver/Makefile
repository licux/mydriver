obj-m = hellokeys.o ledRGB_platform.o ledRGB_class_platform.o led_UIO_platform.o
ARCH := arm
KERNEL_DIR ?= ../../../linux
CROSS_COMPILE ?= ../../../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-

all:
	make -C $(KERNEL_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(PWD)/$(CROSS_COMPILE) M=$(PWD) modules

clean:
	make -C $(KERNEL_DIR) ARCH=$(ARCH) CROSS_COMPILE=$(PWD)/$(CROSS_COMPILE) M=$(PWD) clean
