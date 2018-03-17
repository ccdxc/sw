#
# Copyright (c) 2017-2018, Pensando Systems Inc.
#

LIBRARY = libpcieport.a libpcieport.so
SOURCES = \
	pcieport.c \
	events.c \
	config.c \
	intr.c \
	fsm.c \
	utils.c \
	portcfg.c \
	serdes.c \
	sbus_pcie_rom.s \
	$(NULL)

# sbus routines from asic
SOURCES += \
	cap_sbus_api.c \
	cap_pcie_api.c \
	$(NULL)

LCFLAGS += \
	-I $(ASIC_ROOT)/capri/design/common \
	-I $(ASIC_ROOT)/capri/model/cap_top \
	-I $(ASIC_ROOT)/capri/model/cap_top/csr_defines \
	-I $(TOPDIR)/include/common \
	-I ../../misc/include \
	-I ../../pciemgrutils/include \
	-I ../../pciemgr/include \
	-I ../../pal/include \
	-I ../include \
	$(NULL)

# Build cap_* asic files with CAPRI_SW
LCFLAGS += -DCAPRI_SW
# Include sbus pcie rom loader
LCFLAGS += -DSBUS_PCIE_ROM
# romfile includes generated rom.bin 
LCFLAGS += -I$(GENDIR)

TOPDIR = ../../../..
include $(TOPDIR)/make/Makefile.inc

LCLEANFILES += $(GENDIR)/sbus_pcie_rom.bin

quiet_cmd_genrom = GEN     $@
      cmd_genrom = ./sbus_rom2bin.py $^ $@

$(GENDIR)/sbus_pcie_rom.o: sbus_pcie_rom.s $(GENDIR)/sbus_pcie_rom.bin

$(GENDIR)/sbus_pcie_rom.bin: serdes.0x108C_2347.rom
	$(call cmd,genrom)
