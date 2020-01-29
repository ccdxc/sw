# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_PREREQS  := apollo_rxdma.p4bin apollo_txdma.p4bin hal.memrgns
MODULE_TARGET   := libnicmgr_apollo.so
MODULE_PIPELINE := apollo
MODULE_INCS     := ${MODULE_SRC_DIR}/../include
MODULE_SRCS     := $(shell find ${MODULE_SRC_DIR} -type f -name '*.cc' \
                   ! -name 'ftl*' \
                   ! -name 'accel*' \
                   ! -name '*iris*')
include ${MKDEFS}/post.mk
