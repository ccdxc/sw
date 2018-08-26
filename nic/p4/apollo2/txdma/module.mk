# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = apollo2_txdma.p4bin
MODULE_SRCS     = apollo2_txdma.p4
MODULE_PIPELINE = apollo2
MODULE_NCC_OPTS = --asm-out --p4-plus --no-ohi --two-byte-profile \
                  --pd-gen --p4-plus-module txdma \
                  --gen-dir ${BLD_GEN_DIR} \
                  --cfg-dir ${BLD_PGMBIN_DIR} \
                  --fe-flags="-I${TOPDIR}"
MODULE_DEPS     = $(shell find ${MODULE_DIR}/ -name '*' -type f) \
                  $(shell find ${TOPDIR}/nic/p4/include -name '*')
include ${MAKEDEFS}/post.mk
