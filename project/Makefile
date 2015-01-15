MODULE_NAME := stats_of_random

EXTRA_CFLAGS += -I$(src)

ifeq ($(KERNELRELEASE),)
# 'Out-of-kernel' part

MODULE_SOURCES := \
	module.c \
	symbols.c \
	kernfunc.c \
	hijacks.c

KBUILD_DIR=$(shell sh ./scripts/find_kernel_src.sh)
UNAME=$(shell uname -r)
PWD := $(shell pwd)

all: $(MODULE_NAME).ko

$(MODULE_NAME).ko: $(MODULE_SOURCES)

	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KBUILD_DIR) M=$(PWD) clean

	rm -f Module* 

.PHONY: all clean

else
# KBuild part. 
# It is used by the kernel build system to actually build the module.
ccflags-y :=  -I$(src)

obj-m := $(MODULE_NAME).o
$(MODULE_NAME)-y := \
	module.o \
	symbols.o \
	kernfunc.o \
	hijacks.o

endif
