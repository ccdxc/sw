# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = tcp_proxy_rxdma.asmbin
MODULE_PREREQS  = proxy.p4bin
MODULE_PIPELINE = iris gft
MODULE_INCS     = ${BLD_P4GEN_DIR}/tcp_proxy_rxdma/asm_out \
                  ${BLD_P4GEN_DIR}/tcp_proxy_rxdma/alt_asm_out \
                  ${MODULE_DIR}/../include \
                  ${MODULE_DIR}/../../include \
                  ${MODULE_DIR}/../../../common-p4+/include \
                  ${MODULE_DIR}/../../../cpu-p4plus/include \
                  ${TOPDIR}/nic/include
MODULE_SRCS     = $(MODULE_DIR)/tcp-ack.s \
                  $(MODULE_DIR)/tcp-ack-slow.s \
                  $(MODULE_DIR)/tcp-rdesc-alloc.s \
                  $(MODULE_DIR)/tcp-read-rnmdr-alloc-idx.s \
                  $(MODULE_DIR)/tcp-read-tx2rx-shared.s \
                  $(MODULE_DIR)/tcp-read-tx2rx-shared-ext.s \
                  $(MODULE_DIR)/tcp-rpage-alloc.s \
                  $(MODULE_DIR)/tcp-rtt.s \
                  $(MODULE_DIR)/tcp-rx-stage1-dummy.s \
                  $(MODULE_DIR)/tcp-rx.s \
                  $(MODULE_DIR)/tcp-fc.s \
                  $(MODULE_DIR)/tcp-rx-stats.s \
                  $(MODULE_DIR)/tcp-slow-write-serq.s \
                  $(MODULE_DIR)/tcp-write-serq.s \
                  $(MODULE_DIR)/tcp-write-serq2.s \
                  $(MODULE_DIR)/tcp-write-serq3.s \
                  $(MODULE_DIR)/tcp-write-arq.s \
                  $(MODULE_DIR)/tcp_ooo_processing_launch_dummy1.s \
                  $(MODULE_DIR)/tcp_ooo_processing_launch_dummy2.s \
                  $(MODULE_DIR)/tcp_ooo_qbase_cb_load.s \
                  $(MODULE_DIR)/tcp_ooq_book_keeping.s

MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR  = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
