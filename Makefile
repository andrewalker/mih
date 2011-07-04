
# States that there is one module to be built from the object file mih_mod.o
# obj-m = mih_mod.o
obj-m += mih_mod.o

# PROGS=mihfd mih_client
PROGS=mihfd

# if KERNELRELEASE is defined, we've been invoked from the
# kernel build system and can use its language.
ifneq ($(KERNELRELEASE),)
	# obj-m := mih.o sysfs_ex.o
	# obj-m := mih_mod.o
	# obj-m +:= mih_mod.o
# Otherwise we were called directly from the command line;
# invoke the kernel build system.
else
	# KERNELDIR ?= /lib/modules/$(shell uname -r)/build
	KERNELDIR := /lib/modules/$(shell uname -r)/build
	# KERNELDIR := /lib/modules/2.6.35.4/build
	PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	# $(MAKE) $(PROGS)
endif

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	# rm -f $(PROGS)

# install:
	# install -d $(INSTALLDIR)
	# install -c $(OBJS) $(INSTALLDIR)

mihfd:	mihfd.c
	gcc -Wall mihfd.c -o mihfd	-l pthread

mih_client:	mih_client.c
	gcc -Wall mih_client.c -o mih_client

