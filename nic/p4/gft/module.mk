# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = gft.p4bin
MODULE_SRCS     = ${MODULE_SRC_DIR}/gft.p4
MODULE_PIPELINE = gft
MODULE_NCC_OPTS = --asm-out --gen-dir ${BLD_P4GEN_DIR} \
                  --cfg-dir ${BLD_PGMBIN_DIR} \
                  --phv-flits 12 --i2e-user --single-pipe \
                  --pipeline ${PIPELINE} \
                  --fe-flags="-I${TOPDIR} -I${SDKDIR}"

MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*')
MODULE_POSTGEN_MK = module_p4pd.mk
include ${MKDEFS}/post.mk
