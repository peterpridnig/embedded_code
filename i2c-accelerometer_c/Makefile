# Copyright (C) 2017, Chris Simmonds (chris@2net.co.uk)
#
# If cross-compiling, CC must point to your cross compiler, for example:
# make CPP=arm-cortex_a8-linux-gnueabihf-g++
# make CPP=arm-buildroot-linux-gnueabihf-g++

LOCAL_CFLAGS ?= -Wall -g
LDLIBS=-lrt GPIO.cpp -I. ads1015.cpp
PROGRAM = i2c-accelerometer

$(PROGRAM): $(PROGRAM).cpp
	$(CPP) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

install : $(PROGRAM)
	scp $(PROGRAM) root@10.0.0.117:/root
	cp $(PROGRAM) $(USERLAND)/code/bin

clean:
	rm -f $(PROGRAM)

