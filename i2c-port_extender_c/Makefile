# If cross-compiling, CC must point to your cross compiler, for example:
# make CC=arm-linux-gnueabihf-gcc

LOCAL_CFLAGS ?= -Wall -g -std=c11
LDLIBS=-lrt
PROGRAM = i2c-port_extender

$(PROGRAM): $(PROGRAM).cpp
	$(CPP) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

install : $(PROGRAM)
	scp $(PROGRAM) root@10.0.0.117:/root
	cp $(PROGRAM) $(USERLAND)/code/bin

clean:
	rm -f $(PROGRAM)

