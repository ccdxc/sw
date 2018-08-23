# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = gft.p4bin
MODULE_SRCS     = gft.p4
MODULE_PIPELINE = gft
MODULE_NCC_OPTS = --asm-out --gen-dir ${BLD_GEN_DIR} \
                  --cfg-dir $(OBJ_DIR)/pgm_bin \
                  --phv-flits 12 --i2e-user --single-pipe \
                  --pipeline ${PIPELINE} \
                  --fe-flags="-I${TOPDIR}"
MODULE_DEPS     = $(shell find ${MODULE_DIR}/ -name '*.p4' -o -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*')
include ${MAKEDEFS}/post.mk
