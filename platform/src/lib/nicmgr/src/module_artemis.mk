# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_PREREQS  := artemis_rxdma.p4bin artemis_txdma.p4bin
MODULE_TARGET   := libnicmgr_artemis.so
MODULE_PIPELINE := artemis
MODULE_INCS     := ${MODULE_SRC_DIR}/../include \
                   ${TOPDIR}/nic/sdk/platform/devapi
MODULE_SRCS     := $(shell find ${MODULE_SRC_DIR} -type f -name '*.cc' \
                   ! -name 'accel*' \
                   ! -name '*iris*')
include ${MKDEFS}/post.mk
