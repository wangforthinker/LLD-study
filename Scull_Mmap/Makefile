ifneq ($(KERNELRELEASE),)
	obj-m := scull_mmap.o
	scull_mmap-objs := scull_main.o scull.o
else
	KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	PWD := $(shell pwd)
default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	rm *.o
endif
