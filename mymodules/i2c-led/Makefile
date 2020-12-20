ARCH := arm
CROSS_COMPILE := ../../../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-
KPATH := ../../../linux

obj-m := i2c-lcd.o

all:
	make ARCH=$(ARCH) CROSS_COMPILE=$(PWD)/$(CROSS_COMPILE) -C $(KPATH) M=$(PWD) modules

clean:
	make ARCH=$(ARCH) CROSS_COMPILE=$(PWD)/$(CROSS_COMPILE) -C $(KPATH) M=$(PWD) clean

    
    