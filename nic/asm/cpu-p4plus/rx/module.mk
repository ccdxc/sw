# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = cpu_p4plus_rxdma.asmbin
MODULE_PREREQS      = cpu_p4plus.p4bin
MODULE_INCS         = ${BLD_GEN_DIR}/cpu_rxdma/asm_out \
                      ${MODULE_DIR}/../include \
                      ${MODULE_DIR}/../../common-p4+/include \
                      ${MODULE_DIR}/../../../include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
