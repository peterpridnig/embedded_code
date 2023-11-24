##################
### makefile
##################

# # comment
#
# target:  dependency1 dependency2 ... dependencyn
#       <tab> command
# 
# # (note: the <tab> in the command line is necessary for make to work)


TARGET = display

all: $(TARGET)

$(TARGET): $(TARGET).sh
	scp $(TARGET).sh root@10.0.0.117:/root
	cp $(TARGET).sh $(USERLAND)/code/bin
clean:
	$(RM) *~

