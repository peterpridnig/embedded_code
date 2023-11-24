ifneq ($(KERNELRELEASE),)
obj-m	:= hd44780.o

else
#KDIR	:= /lib/modules/$(shell uname -r)/build
KDIR    := $(WORKAREA)/buildroot/output/build/linux-5.15.133/
PWD	:= $(shell pwd)

default:
	$(MAKE)	-C $(KDIR) M=$(PWD) modules
endif

install : hd44780.ko
	cp hd44780.ko ../../bin
	scp hd44780.ko root@10.0.0.117:/root

clean:
	rm -rf *.ko *.o .*.cmd .tmp_versions Module.symvers *.mod *.dwo *~
	rm -rf modules.order *.mod.c
