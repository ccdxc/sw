# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = rdma_resp_txdma.asmbin
MODULE_PREREQS  = rdma.p4bin
MODULE_PIPELINE = iris gft
MODULE_INCS     = ${BLD_P4GEN_DIR}/rdma_resp_txdma/asm_out \
                  ${MODULE_DIR}/../common/include \
                  ${MODULE_DIR}//include \
                  ${TOPDIR}/nic/asm/common-p4+/include \
                  ${TOPDIR}/nic/p4/include \
                  ${TOPDIR}/nic/include \
                  ${TOPDIR}/nic
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR  = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
