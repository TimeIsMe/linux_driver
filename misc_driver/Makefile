KERNELDIR := /home/ml/linux/linux_kernel
CURRENT_PATH := $(shell pwd)
CC := arm-linux-gnueabihf-gcc
obj-m := miscled.o
app-obj := miscLedApp

build: kernel_module app-obj

kernel_module:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) modules

app-obj:
	$(CC) -o miscLedApp miscLedApp.c

clean:
	$(MAKE) -C $(KERNELDIR) M=$(CURRENT_PATH) clean
	rm miscLedApp