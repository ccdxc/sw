# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = apollo_rxdma.p4bin
MODULE_SRCS     = ${MODULE_SRC_DIR}/apollo_rxdma.p4
MODULE_PIPELINE = apollo
MODULE_NCC_OPTS = --asm-out --p4-plus --no-ohi --two-byte-profile \
                  --p4-plus-module rxdma \
                  --gen-dir ${BLD_P4GEN_DIR} \
                  --cfg-dir ${BLD_PGMBIN_DIR}/apollo_rxdma \
                  --fe-flags="-I${TOPDIR}"
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h') \
                  $(shell find ${TOPDIR}/nic/p4/include -name '*')
MODULE_POSTGEN_MK = module_p4pd.mk
include ${MKDEFS}/post.mk
