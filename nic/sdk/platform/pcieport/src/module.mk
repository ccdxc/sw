# {C} Copyright 2018,2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := libpcieport.lib
MODULE_INCS     := ${MODULE_SRC_DIR}/${ASIC}
MODULE_SRCS     := $(wildcard ${MODULE_SRC_DIR}/*.c) \
                   $(wildcard ${MODULE_SRC_DIR}/${ASIC}/*.c)
MODULE_PREREQS  := sbus_pcie_rom_$(ASIC).submake
MODULE_LDFLAGS  := ${BLD_OUT_DIR}/sbus_pcie_rom_$(ASIC)_submake/sbus_pcie_rom.o
ifeq ($(ASIC),capri)
MODULE_DEFS     := -DCAPRI_SW
endif
include ${MKDEFS}/post.mk
