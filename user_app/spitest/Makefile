TARGET = adc
SRCS = adc.cpp mcp320x.cpp libspi.cpp
OBJS = $(subst .cpp,.o,$(SRCS))

RM := rm
C++ := ../../tools/arm-bcm2708/gcc-linaro-arm-linux-gnueabihf-raspbian/bin/arm-linux-gnueabihf-g++

CPPFLAGS = -g -fPIC -O2 

%.o : %.cpp
	$(C++) -c $<

$(TARGET): $(OBJS)
	$(C++) $(CPPFLAGS) -o $@ $^

.PHONY: clean

clean:
	$(RM) $(OBJS) $(TARGET)
	