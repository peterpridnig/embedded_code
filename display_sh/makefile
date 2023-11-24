##################
### makefile
##################

# # comment
#
# target:  dependency1 dependency2 ... dependencyn
#       <tab> command
# 
# # (note: the <tab> in the command line is necessary for make to work)


TARGET = test-display-pins display

all: test-display-pins.sh display.sh
	scp display.sh root@10.0.0.117:/root
	cp display.sh $(USERLAND)/code/bin
	scp test-display-pins.sh root@10.0.0.117:/root
	cp test-display-pins.sh $(USERLAND)/code/bin

clean:
	$(RM) *~

