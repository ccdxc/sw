# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := libpcietlp.so
MODULE_INCS     := ${MODULE_SRC_DIR}/../include/ \
                    ${TOPDIR}/capri/design/common \
                    ${TOPDIR}/nic/asic/capri/model/cap_top \
                    ${TOPDIR}/nic/asic/capri/model/cap_top/csr_defines
include ${MKDEFS}/post.mk
