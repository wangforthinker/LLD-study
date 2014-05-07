ifneq ($(KERNELRELEASE),)
	obj-m := sculls.o
	sculls-objs := scull.o scull_set.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	rm *.o
endif
