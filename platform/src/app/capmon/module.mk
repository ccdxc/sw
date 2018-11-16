# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := capmon.bin
MODULE_SOLIBS   := pal
MODULE_INCS     := ${TOPDIR}/nic/asic/capri/model/cap_top \
                    ${TOPDIR}/nic/asic/capri/model/cap_top/csr_defines
include ${MKDEFS}/post.mk
