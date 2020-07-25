KERNELDIR := /home/ml/linux/linux_kernel
CURRENT_PATH := $(shell pwd)
CC := arm-linux-gnueabihf-gcc
obj-m := leddevice.o
obj-m += leddriver.o
app-obj := ledApp

build: kernel_modules app-obj
kernel_modules:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules
app-obj:
	$(CC) -o ledApp ledApp.c
clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
	rm ledApp