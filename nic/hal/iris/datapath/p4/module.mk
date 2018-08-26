# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = iris.p4bin
MODULE_SRCS     = p4.p4
MODULE_PIPELINE = iris
MODULE_NCC_OPTS = --asm-out --pd-gen --gen-dir ${BLD_GEN_DIR} \
                  --cfg-dir ${BLD_PGMBIN_DIR} \
                  --split-deparse-only-headers \
                  --pipeline ${PIPELINE} \
                  --fe-flags="-I${TOPDIR}"
MODULE_DEPS     = $(shell find ${MODULE_DIR}/ -name '*' -type f)
include ${MAKEDEFS}/post.mk
