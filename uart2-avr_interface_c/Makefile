# pridnig, 27.12.2023
#
# If cross-compiling, CC must point to the compiler:
# make CPP=arm-linux-gnueabihf-g++

LOCAL_CFLAGS ?= -Wall -g
LDLIBS=-lrt -I. GPIO.cpp
PROGRAM = uart2-avr_interface

$(PROGRAM): $(PROGRAM).cpp
	$(CPP) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

install : $(PROGRAM)
	scp $(PROGRAM) root@10.0.0.117:/root
	scp flash_avr.sh root@10.0.0.117:/root
	scp test-attiny-pins.sh root@10.0.0.117:/root
	scp Interface_hex/Interface.hex root@10.0.0.117:/root
	cp $(PROGRAM) ../bin
	cp flash_avr.sh ../bin
	cp test-attiny-pins.sh ../bin
	cp Interface_hex/Interface.hex ../bin

clean:
	rm -f $(PROGRAM)
	rm -f *~

