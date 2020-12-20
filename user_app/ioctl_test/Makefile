TARGET = ioctl_test
SRCS = ioctl_test.c
OBJS = $(subst .cpp,.o,$(SRCS))

RM := rm
CC := ../../../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-gcc

CPPFLAGS = -g -fPIC -O2 

%.o : %.cpp
	$(CC) -c $<

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) -o $@ $^

.PHONY: clean

clean:
	$(RM) $(OBJS) $(TARGET)
	