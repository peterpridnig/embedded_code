# If cross-compiling, CC must point to your cross compiler, for example:
# make CPP=arm-buildroot-linux-gnueabihf-g++

LOCAL_CFLAGS ?= -Wall -g
LDLIBS = minmea.c -lncurses
PROGRAM = uart4-readgps

$(PROGRAM): $(PROGRAM).cpp
	$(CPP) $(CFLAGS) $(LOCAL_CFLAGS) $(LDLIBS) $(LDFLAGS) $^ -o $@

install : $(PROGRAM)
	scp $(PROGRAM) root@10.0.0.117:/root
	cp $(PROGRAM) $(USERLAND)/code/bin

clean:
	rm -f $(PROGRAM)
	rm *~
	rm a.out

