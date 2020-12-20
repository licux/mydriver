TARGET = librpi_gpio.a
SRCS = rpi_gpiolib.c
OBJS = $(subst .c,.o,$(SRCS))
HEADERS = rpi_gpiolib.h rpi_gpio.hello

RM := rm
CC := ../../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-gcc
AR := ../../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-ar

CPPFLAGS = -g -fPIC -O2 -l./
LDFLAGS = -g -fPIC
ARFLAGS = cr

$(TARGET): $(OBJS)
	$(AR) $(ARFLAGS) $@ $^ ${LOADLIBES}

.PHONY: clean

clean:
	$(RM) $(OBJS) $(TARGET)
	