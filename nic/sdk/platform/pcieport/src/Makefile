#
# Copyright (c) 2017-2019, Pensando Systems Inc.
#

include ${MKINFRA}/config_${ARCH}.mk

MODULE_BIN_DIR := ${BLD_OUT_DIR}/sbus_pcie_rom_submake

all: printvars ${MODULE_BIN_DIR}/sbus_pcie_rom.o

${MODULE_BIN_DIR}/sbus_pcie_rom.o: sbus_pcie_rom.s ${MODULE_BIN_DIR}/sbus_pcie_rom.bin
	${CMD_AS} -I${MODULE_BIN_DIR} sbus_pcie_rom.s -o ${MODULE_BIN_DIR}/sbus_pcie_rom.o

${MODULE_BIN_DIR}/sbus_pcie_rom.bin: serdes.0x1094_2347.rom
	./sbus_rom2bin.py serdes.0x1094_2347.rom ${MODULE_BIN_DIR}/sbus_pcie_rom.bin

.PHONY: printvars
printvars:
	$(foreach v, $(.VARIABLES), $(info $(v) = $($(v))))
