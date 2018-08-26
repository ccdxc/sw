# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = common_p4plus.asmbin
MODULE_PREREQS      = common_p4plus_rxdma.p4bin common_p4plus_txdma.p4bin
MODULE_INCS         = ${BLD_GEN_DIR}
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
