TARGET = libi2clcd.a
SRCS = libi2clcd.c
OBJS = $(subst .c,.o,$(SRCS))
HEADERS = i2c_lcd.h libi2clcd.h

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
	