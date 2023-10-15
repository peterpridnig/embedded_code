##################
### makefile
##################

# # comment
#
# target:  dependency1 dependency2 ... dependencyn
#       <tab> command
# 
# # (note: the <tab> in the command line is necessary for make to work)


TARGET = led_blink

all: $(TARGET)

$(TARGET): $(TARGET).sh
	cp $(TARGET).sh ../../rootfs_nfs/root/$(TARGET).sh
	chmod 760 ../../rootfs_nfs/root/$(TARGET).sh
clean:
	$(RM) *~

