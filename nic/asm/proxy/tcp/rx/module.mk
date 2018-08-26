# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = tcp_proxy_rxdma.asmbin
MODULE_PREREQS  = proxy.p4bin
MODULE_INCS     = ${BLD_GEN_DIR}/tcp_proxy_rxdma/asm_out \
                  ${BLD_GEN_DIR}/tcp_proxy_rxdma/alt_asm_out \
                  ${MODULE_DIR}/../include \
                  ${MODULE_DIR}/../../include \
                  ${MODULE_DIR}/../../../common-p4+/include \
                  ${MODULE_DIR}/../../../cpu-p4plus/include \
                  ${TOPDIR}/nic/include
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
