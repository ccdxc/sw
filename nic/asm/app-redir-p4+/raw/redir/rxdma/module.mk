# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = rawr_rxdma.asmbin
MODULE_PREREQS      = app_redir_p4plus.p4bin
MODULE_INCS         = ${BLD_GEN_DIR}/rawr_rxdma/asm_out \
                      ${TOPDIR}/nic/asm/app-redir-p4+/common/include \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/p4/include \
                      ${TOPDIR}/nic/include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
