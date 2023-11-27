# Copyright (C) 2017, Chris Simmonds (chris@2net.co.uk)
#
# If cross-compiling, CC must point to your cross compiler, for example:
# make CC=arm-linux-gnueabihf-gcc

LOCAL_CFLAGS ?= -Wall -g
LDLIBS=-lrt GPIO.cpp -I. ads1015.cpp
PROGRAM = accelerometer

$(PROGRAM): $(PROGRAM).cpp
	$(CPP) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

install : $(PROGRAM)
	scp $(PROGRAM) root@10.0.0.117:/root
	cp $(PROGRAM) ../bin

clean:
	rm -f $(PROGRAM)
	rm *~

