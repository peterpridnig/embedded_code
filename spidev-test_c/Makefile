# Copyright (C) 2017, Chris Simmonds (chris@2net.co.uk)
#
# If cross-compiling, CC must point to your cross compiler, for example:
# make CC=arm-linux-gnueabihf-gcc

LOCAL_CFLAGS ?= -Wall -g
LDLIBS=-lrt -I. 
PROGRAM = spidev_test

$(PROGRAM): $(PROGRAM).c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

install : $(PROGRAM)
	scp $(PROGRAM) root@10.0.0.117:/root
	scp readme_spidevtest.md root@10.0.0.117:/root
	cp $(PROGRAM) ../bin

clean:
	rm -f $(PROGRAM)
	rm *~

