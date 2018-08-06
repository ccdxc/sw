CUR_DIR:=$(PWD)
STORAGE_DIR:=$(shell dirname ${CUR_DIR})
SW_DIR:=$(shell dirname ${STORAGE_DIR})
CUR_USER:=$(shell whoami)
CUR_TIME:=$(shell date +%Y-%m-%d_%H.%M.%S)
CONTAINER_NAME:=${CUR_USER}_${CUR_TIME}

# get a shell with the dependencies image loaded, with the host filesystem mounted.

MODNAME = sonic
INCDIRS = ./../../../../../../nic/include ./../../../osal ./../../../../include

$(MODNAME)-y = sonic_bus_pci.o
$(MODNAME)-y += sonic_dev.o
$(MODNAME)-y += sonic_lif.o
$(MODNAME)-y += sonic_main.o
$(MODNAME)-y += ./../../../osal/linux/kernel/osal_sys.o
$(MODNAME)-y += ./../../../osal/linux/kernel/osal_logger.o

obj-m = $(MODNAME).o
ccflags-y = $(INCDIRS:%=-I$(PWD)/%) -Wno-implicit-function-declaration
KSRC ?= /lib/modules/$(shell uname -r)/build
modules: modules-clean
	make -C $(KSRC) M=$(PWD) modules

modules-clean:
	make -C $(KSRC) M=$(PWD) clean

