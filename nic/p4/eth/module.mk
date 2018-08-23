# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = eth.p4bin
MODULE_SRCS     = eth_rxdma_actions.p4 eth_txdma_actions.p4 virtio_txdma.p4 virtio_rxdma.p4
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi \
                  --two-byte-profile --fe-flags="-I${TOPDIR}" \
				  --gen-dir ${BLD_GEN_DIR}
MODULE_DEPS     = $(shell find ${MODULE_DIR}/ -name '*.p4') \
                  $(shell find ${MODULE_DIR}/ -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*') \
				  $(shell find ${MODULE_DIR}/../common-p4+ -name '*.p4')
include ${MAKEDEFS}/post.mk
