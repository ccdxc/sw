# {C} Copyright 2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = mpu_timestamp_$(PIPELINE).asmbin
MODULE_PREREQS      = scanner_session.p4bin
MODULE_PIPELINE     = athena
MODULE_INCS         = ${BLD_P4GEN_DIR}/mpu_timestamp/asm_out \
                      ${BLD_P4GEN_DIR}/mpu_timestamp/alt_asm_out \
                      ${TOPDIR}/nic/p4/ftl_dev/include \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/p4/include \
                      ${TOPDIR}/nic/include \
                      ${MODULE_DIR}/..
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm_txdma
include ${MKDEFS}/post.mk
