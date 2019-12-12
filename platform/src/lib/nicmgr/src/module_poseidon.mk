# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_PREREQS  := poseidon_rxdma.p4bin poseidon_txdma.p4bin
MODULE_TARGET   := libnicmgr_poseidon.so
MODULE_PIPELINE := poseidon
MODULE_INCS     := ${MODULE_SRC_DIR}/../include \
                   ${TOPDIR}/nic/sdk/platform/devapi
MODULE_SRCS     := $(shell find ${MODULE_SRC_DIR} -type f -name '*.cc' \
                   ! -name 'accel*' \
                   ! -name '*iris*')
include ${MKDEFS}/post.mk
