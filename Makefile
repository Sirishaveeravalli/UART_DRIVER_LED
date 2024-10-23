# Makefile for UART LED kernel module and user-space programs

# Kernel build options
KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

# Target executables
TARGETS := read write

# Default target
all: module user

# Build kernel module
module:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Build user-space programs
user: $(TARGETS)

read: uart_read.c
	gcc -o read uart_read.c

write: uart_write.c
	gcc -o write uart_write.c

# Clean the build
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f $(TARGETS)

# Unload and clean the device file
unload:
	sudo rmmod Driver
	sudo rm -f /dev/devUART0

# Load the kernel module and create device file
load:
	sudo insmod Driver.ko
	sudo mknod /dev/devUART0 c 89 0
	sudo chmod 666 /dev/devUART0

.PHONY: all module user clean unload load

