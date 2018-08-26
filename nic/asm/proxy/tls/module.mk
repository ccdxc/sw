# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = tls.asmbin
MODULE_PREREQS  = proxy.p4bin
MODULE_INCS     = ${BLD_GEN_DIR}/tls_txdma_pre_crypto_enc/asm_out \
                  ${MODULE_DIR}/include \
                  ${MODULE_DIR}/../include \
                  ${MODULE_DIR}/../../common-p4+/include \
                  ${MODULE_DIR}/../../cpu-p4plus/include \
                  ${MODULE_DIR}/../tcp/include \
                  ${TOPDIR}/nic/include
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
