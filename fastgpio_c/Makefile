ifneq ($(KERNELRELEASE),)
obj-m	:= fastgpio.o

else
#KDIR	:= /lib/modules/$(shell uname -r)/build
KDIR    := $(WORKAREA)/buildroot/output/build/linux-5.15.133/
PWD	:= $(shell pwd)

default:
	$(MAKE)	-C $(KDIR) M=$(PWD) modules
endif

install : fastgpio.ko
	scp fastgpio.ko root@10.0.0.117:/root

clean:
	rm -rf *.ko *.o .*.cmd .tmp_versions Module.symvers
	rm -rf modules.order *.mod.c
