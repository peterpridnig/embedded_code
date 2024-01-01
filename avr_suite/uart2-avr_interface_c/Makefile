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
	scp read-efuses_avr.sh root@10.0.0.117:/root
	scp write-efuses_avr.sh root@10.0.0.117:/root
	scp test-attiny-pins.sh root@10.0.0.117:/root
	scp Interface_hex/Interface.hex root@10.0.0.117:/root
	cp $(PROGRAM) $WORKAREA/userland/code/bin
	cp flash_avr.sh $WORKAREA/userland/code/bin
	cp test-attiny-pins.sh $WORKAREA/userland/code/bin
	cp Interface_hex/Interface.hex $WORKAREA/userland/code/bin

clean:
	rm -f $(PROGRAM)
	rm -f *~

