# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = eth_virtio_rx.asmbin
MODULE_PREREQS      = eth.p4bin
MODULE_INCS         = ${BLD_GEN_DIR}/virtio_rxdma/asm_out \
                      ${BLD_GEN_DIR}/virtio_rxdma/alt_asm_out \
                      ${MODULE_DIR}/.. \
                      ${TOPDIR}/nic/asm/common-p4+/include \
                      ${TOPDIR}/nic/p4/include \
                      ${TOPDIR}/nic/include 
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
