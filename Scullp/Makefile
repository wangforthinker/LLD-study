ifneq ($(KERNELRELEASE),)
	obj-m := scullp.o
	scullp-objs := scull.o scull_main.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	rm *.o
endif
