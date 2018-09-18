# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = apollo_p4.p4bin
MODULE_SRCS     = apollo.p4
MODULE_PIPELINE = apollo
MODULE_NCC_OPTS = --asm-out --gen-dir ${BLD_GEN_DIR} \
                  --cfg-dir ${BLD_PGMBIN_DIR} \
                  --i2e-user --fe-flags="-I${TOPDIR}"
MODULE_DEPS     = $(shell find ${MODULE_DIR}/ -name '*' -type f) \
                  $(shell find ${TOPDIR}/nic/p4/include -name '*')
include ${MAKEDEFS}/post.mk
