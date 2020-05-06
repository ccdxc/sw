#
# Copyright (c) 2017-2020, Pensando Systems Inc.
#

include ${MKINFRA}/config_${ARCH}.mk

MODULE_BIN_DIR := ${BLD_OUT_DIR}/sbus_pcie_rom_elba_submake

# ELBA-TODO placeholder serdesfw rom
pcie_romvers = \
	0x10AA_2347 \
	$(NULL)

pcie_rombins = $(foreach v,$(pcie_romvers),${MODULE_BIN_DIR}/serdes.$(v).bin)

all: ${MODULE_BIN_DIR}/sbus_pcie_rom.o

${MODULE_BIN_DIR}/sbus_pcie_rom.o: sbus_pcie_rom.s $(pcie_rombins)
	${CMD_AS} -o $@ -I${MODULE_BIN_DIR} $<

${MODULE_BIN_DIR}/%.bin: %.rom sbus_rom2bin.py
	./sbus_rom2bin.py $< $@
