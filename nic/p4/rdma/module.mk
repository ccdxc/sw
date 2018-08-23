# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = rdma.p4bin

MODULE_SRCS     = rdma_rxdma.p4 \
                  rdma_resp_rxdma.p4 \
                  rdma_resp_txdma.p4 \
                  rdma_req_rxdma.p4 \
                  rdma_req_txdma.p4 \
                  rdma_cq_rxdma.p4 \
                  rdma_cq_txdma.p4 \
                  rdma_aq_txdma.p4 \
                  rdma_aq_rxdma.p4

MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi \
                  --two-byte-profile --fe-flags="-I${TOPDIR}" \
                  --gen-dir ${BLD_GEN_DIR}

MODULE_DEPS     = $(shell find ${MODULE_DIR}/ -name '*.p4' -o -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*')
include ${MAKEDEFS}/post.mk
