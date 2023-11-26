# If cross-compiling, CC must point to your cross compiler, for example:
# make CC=arm-linux-gnueabihf-gcc
#pridnig, 26.11.2023

LOCAL_CFLAGS ?= -Wall -g
LDLIBS= -I. parselib.c
PROGRAM = parse

$(PROGRAM): $(PROGRAM).c
	gcc $(CFLAGS) $(LOCAL_CFLAGS) $(LDLIBS) $^ -o $@

clean:
	rm -f $(PROGRAM)
	rm *~

