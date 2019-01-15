# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = tls_txdma_pre_crypto_enc.asmbin
MODULE_PREREQS  = tls.p4bin
MODULE_PIPELINE = iris gft
MODULE_INCS     = ${BLD_P4GEN_DIR}/tls_txdma_pre_crypto_enc/asm_out \
                  ${MODULE_DIR}/include \
                  ${MODULE_DIR}/../../include \
                  ${MODULE_DIR}/../../../common-p4+/include \
                  ${MODULE_DIR}/../../../cpu-p4plus/include \
                  ${MODULE_DIR}/../../tls/include \
                  ${TOPDIR}/nic/include
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_SRCS     = $(MODULE_DIR)/tls-enc-serq.s \
                  $(MODULE_DIR)/tls-enc-read-serq-entry.s \
                  $(MODULE_DIR)/tls-enc-rx-serq.s \
                  $(MODULE_DIR)/tls-enc-read-tnmdr-alloc-idx.s \
                  $(MODULE_DIR)/tls-enc-read-pkt-descr.s \
                  $(MODULE_DIR)/tls-enc-serq-consume.s \
                  $(MODULE_DIR)/tls-enc-tdesc-alloc.s \
                  $(MODULE_DIR)/tls-enc-tpage-alloc.s \
                  $(MODULE_DIR)/tls-enc-bld-barco-req.s \
                  $(MODULE_DIR)/tls-enc-read-barco-pi.s \
                  $(MODULE_DIR)/tls-enc-queue-brq.s \
                  $(MODULE_DIR)/tls-enc-pre-crypto-stats.s \
                  $(MODULE_DIR)/tls-enc-bubble.s \
                  $(MODULE_DIR)/tls-enc-queue-brq-mpp.s \
                  $(MODULE_DIR)/tls-enc-read-random-iv.s

MODULE_BIN_DIR  = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
