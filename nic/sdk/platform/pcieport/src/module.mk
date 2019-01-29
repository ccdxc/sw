# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_PREREQS  := sbus_pcie_rom.submake
MODULE_TARGET   := libpcieport.so
MODULE_INCS     := ${MODULE_SRC_DIR}/../include/ \
                    ${TOPDIR}/capri/design/common \
                    ${TOPDIR}/nic/asic/capri/model/cap_top \
                    ${TOPDIR}/nic/asic/capri/model/cap_top/csr_defines
MODULE_DEFS     := -DCAPRI_SW
MODULE_LDFLAGS  := ${BLD_OUT_DIR}/sbus_pcie_rom_submake/sbus_pcie_rom.o
include ${MKDEFS}/post.mk
