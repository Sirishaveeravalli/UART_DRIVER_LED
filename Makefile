# Define the cross-compiler (if you're cross-compiling for Raspberry Pi/BeagleBone)
# CROSS_COMPILE ?= arm-linux-gnueabihf-

# Kernel source directory
KDIR := /lib/modules/$(shell uname -r)/build

# Module name
MODULE_NAME := uart_rgb_led

# Source files
MODULE_SRC := driver.c
USER_SRC := user_program.c

# Compiler options
CC := $(CROSS_COMPILE)gcc
CFLAGS := -Wall

# Default target
all: $(MODULE_NAME).ko user_program

# Compile kernel module
$(MODULE_NAME).ko: $(MODULE_SRC)
	$(MAKE) -C $(KDIR) M=$(PWD) modules

# Compile user-space application
user_program: $(USER_SRC)
	$(CC) $(CFLAGS) -o user_program $(USER_SRC)

# Clean up object files and the module
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) clean
	rm -f user_program

# Load the module
load:
	sudo insmod $(MODULE_NAME).ko

# Unload the module
unload:
	sudo rmmod $(MODULE_NAME)

# Display kernel log
dmesg:
	dmesg | tail -n 20

.PHONY: all clean load unload dmesg
