KERNELDIR	:=	/usr/src/linux-headers-4.15.0-91-generic
CURRENT_PATH	:=	$(shell pwd)
obj-m	:=	chrdev.o

build:	kernel_modules

kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules 

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean 