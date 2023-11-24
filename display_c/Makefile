# Copyright (C) 2017, Chris Simmonds (chris@2net.co.uk)
#
# If cross-compiling, CC must point to your cross compiler, for example:
# make CC=arm-linux-gnueabihf-gcc

LOCAL_CFLAGS ?= -Wall -g
LDLIBS=-lrt
PROGRAM = display

$(PROGRAM): $(PROGRAM).cpp
	$(CPP) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

install : $(PROGRAM)
	scp $(PROGRAM) root@10.0.0.117:/root

clean:
	rm -f $(PROGRAM)

