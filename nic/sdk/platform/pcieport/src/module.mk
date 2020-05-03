# {C} Copyright 2018,2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := libpcieport.lib
MODULE_PREREQS  := sbus_pcie_rom.submake
MODULE_INCS     := ${MODULE_SRC_DIR}/../include
MODULE_DEFS     := -DCAPRI_SW
MODULE_LDFLAGS  := ${BLD_OUT_DIR}/sbus_pcie_rom_submake/sbus_pcie_rom.o
include ${MKDEFS}/post.mk
