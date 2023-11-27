LOCAL_CFLAGS ?= -Wall -g
#LDLIBS=-lrt
CFLAGS  = -pthread
PROGRAM = thread

$(PROGRAM): $(PROGRAM).cpp
	g++ $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $(LDLIBS) $^ -o $@

install : $(PROGRAM)
	scp $(PROGRAM) root@10.0.0.117:/root

clean:
	rm -f $(PROGRAM)
	rm *~

