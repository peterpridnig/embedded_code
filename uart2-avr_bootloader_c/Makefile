# pridnig, 27.12.2023
#
# If cross-compiling, CC must point to the compiler:
# make CPP=arm-linux-gnueabihf-g++

LOCAL_CFLAGS ?= -Wall -g
LDLIBS=-lrt -I. GPIO.cpp
PROGRAM = uart2-avr_bootloader

$(PROGRAM): $(PROGRAM).cpp
	$(CPP) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

install : $(PROGRAM)
	scp $(PROGRAM) root@10.0.0.117:/root
	scp Init_hex/Init.hex root@10.0.0.117:/root
	cp $(PROGRAM) ../bin
	cp Init_hex/Init.hex ../bin

clean:
	rm -f $(PROGRAM)
	rm -f *~

