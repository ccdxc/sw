# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = common_p4plus_txdma.p4bin
MODULE_SRCS     = common_txdma_actions.p4
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi \
                  --two-byte-profile --fe-flags="-I${TOPDIR}" \
				  --gen-dir ${BLD_GEN_DIR} \
				  --cfg-dir ${BLD_OBJ_DIR} \
				  --p4-plus-module txdma
MODULE_DEPS     = $(shell find ${MODULE_DIR}/ -name '*.p4') \
                  $(shell find ${MODULE_DIR}/ -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*') \
include ${MAKEDEFS}/post.mk
