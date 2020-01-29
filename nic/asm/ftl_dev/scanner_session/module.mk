# {C} Copyright 2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = scanner_session.asmbin
MODULE_PREREQS      = scanner_session.p4bin
MODULE_PIPELINE     = athena
MODULE_INCS         = ${BLD_P4GEN_DIR}/scanner_session/asm_out \
                      ${TOPDIR}/nic/p4/ftl_dev/include \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/p4/include \
                      ${TOPDIR}/nic/include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_txdma
include ${MKDEFS}/post.mk
