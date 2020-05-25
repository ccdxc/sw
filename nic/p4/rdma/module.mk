# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = rdma.p4bin
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/rdma_rxdma.p4 \
                  ${MODULE_SRC_DIR}/rdma_resp_rxdma.p4 \
                  ${MODULE_SRC_DIR}/rdma_resp_txdma.p4 \
                  ${MODULE_SRC_DIR}/rdma_req_rxdma.p4 \
                  ${MODULE_SRC_DIR}/rdma_req_txdma.p4 \
                  ${MODULE_SRC_DIR}/rdma_cq_rxdma.p4 \
                  ${MODULE_SRC_DIR}/rdma_cq_txdma.p4 \
                  ${MODULE_SRC_DIR}/rdma_aq_txdma.p4 \
                  ${MODULE_SRC_DIR}/rdma_aq_rxdma.p4

ifeq ($(ASIC),elba)
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi --asic elba \
                  --two-byte-profile --fe-flags="-DELBA -I${TOPDIR} -I${SDKDIR}" \
                  --gen-dir ${BLD_P4GEN_DIR}
else
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi \
                  --two-byte-profile --fe-flags="-I${TOPDIR} -I${SDKDIR}" \
                  --gen-dir ${BLD_P4GEN_DIR}
endif

MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*')
include ${MKDEFS}/post.mk
