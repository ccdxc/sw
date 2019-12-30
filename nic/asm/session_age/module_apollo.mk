# {C} Copyright 2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = session_age_$(PIPELINE).asmbin
MODULE_PREREQS      = session_age.p4bin
MODULE_PIPELINE     = athena
MODULE_INCS         = ${BLD_P4GEN_DIR}/session_age/asm_out \
                      ${BLD_P4GEN_DIR}/session_age/alt_asm_out \
                      ${TOPDIR}/nic/asm/session_age/common/include \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/p4/include \
                      ${TOPDIR}/nic/include \
                      ${MODULE_DIR}/..
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_txdma
include ${MKDEFS}/post.mk
