# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := libpciemgr.so
MODULE_INCS     := ${TOPDIR}/nic/asic/capri/model/cap_top \
                    ${TOPDIR}/nic/asic/capri/model/cap_top/csr_defines
MODULE_PREREQS  := hal.memrgns
include ${MKDEFS}/post.mk
