KERNEL_VERSION	:= `uname -r`
#KERNEL_DIR	:= /lib/modules/$(KERNEL_VERSION)/build
KERNEL_DIR      := /work/CNXTv20/kernel/output
CROSS_COMPILE   := arm-linux-


PWD		:= $(shell pwd)

obj-m		:= amba.o
amba-objs   	:= amba_usb.o

all: amba

amba:
	@echo "Building Amba USB driver..."
	@(make -C $(KERNEL_DIR) M=$(PWD) CROSS_COMPILE=$(CROSS_COMPILE) modules)

clean:
	-rm -f *.o *.ko .*.cmd .*.flags *.mod.c Module.symvers modules.order tags
	-rm -rf .tmp_versions

