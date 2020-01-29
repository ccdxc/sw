# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_PREREQS  := apulu_rxdma.p4bin apulu_txdma.p4bin hal.memrgns
MODULE_TARGET   := libnicmgr_apulu.so
MODULE_PIPELINE := apulu
MODULE_INCS     := ${MODULE_SRC_DIR}/../include
MODULE_SRCS     := $(shell find ${MODULE_SRC_DIR} -type f -name '*.cc' \
                   ! -name 'ftl*' \
                   ! -name 'accel*' \
                   ! -name '*iris*')
include ${MKDEFS}/post.mk
