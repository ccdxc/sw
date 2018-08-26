# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = smbdc_req_txdma.asmbin
MODULE_PREREQS  = smbdc.p4bin
MODULE_INCS     = ${BLD_GEN_DIR}/smbdc_req_txdma/asm_out \
                  ${MODULE_DIR}/../common/include \
                  ${MODULE_DIR}/include \
                  ${TOPDIR}/nic/p4/include \
                  ${TOPDIR}/nic/include \
                  ${TOPDIR}/nic
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
