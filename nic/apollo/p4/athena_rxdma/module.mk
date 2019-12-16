# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = athena_rxdma.p4bin
MODULE_SRCS     = ${MODULE_SRC_DIR}/p4plus_rxdma.p4
MODULE_PIPELINE = athena
MODULE_NCC_OPTS = --pipeline=athena --asm-out --p4-plus --no-ohi --two-byte-profile \
                  --p4-plus-module rxdma \
                  --gen-dir ${BLD_P4GEN_DIR} \
                  --cfg-dir ${BLD_PGMBIN_DIR}/athena_rxdma \
                  --fe-flags="-I${TOPDIR} -I${SDKDIR}"
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*.p4' -o -name '*.h') \
                  $(shell find ${TOPDIR}/nic/p4/include -name '*')
MODULE_POSTGEN_MK = module_p4pd.mk
include ${MKDEFS}/post.mk
