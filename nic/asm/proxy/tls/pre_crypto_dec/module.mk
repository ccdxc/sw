# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = tls_txdma_pre_crypto_dec.asmbin
MODULE_PREREQS  = tls.p4bin
MODULE_PIPELINE = iris gft
MODULE_INCS     = ${BLD_P4GEN_DIR}/tls_txdma_pre_crypto_dec/asm_out \
                  ${MODULE_DIR}/include \
                  ${MODULE_DIR}/../../include \
                  ${MODULE_DIR}/../../../common-p4+/include \
                  ${MODULE_DIR}/../../../cpu-p4plus/include \
                  ${MODULE_DIR}/../../tls/include \
                  ${TOPDIR}/nic/include
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_SRCS     = $(MODULE_DIR)/tls-dec-serq.s \
                  $(MODULE_DIR)/tls-dec-read-serq-entry.s \
                  $(MODULE_DIR)/tls-dec-rx-serq.s \
                  $(MODULE_DIR)/tls-dec-read-tnmdr-alloc-idx.s \
                  $(MODULE_DIR)/tls-dec-read-header.s \
                  $(MODULE_DIR)/tls-dec-tdesc-alloc.s \
                  $(MODULE_DIR)/tls-dec-tpage-alloc.s \
                  $(MODULE_DIR)/tls-dec-read-barco-pi.s \
                  $(MODULE_DIR)/tls-dec-bld-barco-req.s \
                  $(MODULE_DIR)/tls-dec-queue-brq.s \
                  $(MODULE_DIR)/tls-dec-write-arq.s \
                  $(MODULE_DIR)/tls-dec-queue-brq-mpp.s

MODULE_BIN_DIR  = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
