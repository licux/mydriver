TARGET = uio_app
SRCS = uio_app.c
OBJS = $(subst .c,.o,$(SRCS))

RM := rm
CC := ../../../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-gcc

CPPFLAGS = -g -fPIC -O2 

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $^

.PHONY: clean

clean:
	$(RM) $(OBJS) $(TARGET)
	