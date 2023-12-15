# Copyright (C) 2017, Chris Simmonds (chris@2net.co.uk)
#
# If cross-compiling, CC must point to your cross compiler, for example:
# make CPP=arm-linux-gnueabihf-gcc

LOCAL_CFLAGS ?= -Wall -g
#LDLIBS=-lrt -I
PROGRAM = uart2-avr_interface

$(PROGRAM): $(PROGRAM).cpp
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $^ -o $@

install : $(PROGRAM)
	scp $(PROGRAM) root@10.0.0.117:/root
	scp flash_avr.sh root@10.0.0.117:/root
	scp test-attiny-pins.sh root@10.0.0.117:/root
	scp Interface_hex/Interface.hex root@10.0.0.117:/root
	cp $(PROGRAM) ../bin
	cp flash_avr.sh ../bin
	cp test-attiny-pins.sh ../bin

clean:
	rm -f $(PROGRAM)

