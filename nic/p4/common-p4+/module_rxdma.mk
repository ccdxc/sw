# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = common_p4plus_rxdma.p4bin
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/common_rxdma_actions.p4
ifeq ($(ASIC),elba)
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi --asic elba \
                  --two-byte-profile --fe-flags="-DELBA -I${TOPDIR} -I${SDKDIR}" \
                  --gen-dir ${BLD_P4GEN_DIR} \
                  --cfg-dir ${BLD_PGMBIN_DIR} \
                  --p4-plus-module rxdma
else
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi \
                  --two-byte-profile --fe-flags="-I${TOPDIR} -I${SDKDIR}" \
                  --gen-dir ${BLD_P4GEN_DIR} \
                  --cfg-dir ${BLD_PGMBIN_DIR} \
                  --p4-plus-module rxdma
endif
MODULE_DEPS     = $(shell find ${MODULE_DIR}/ -name '*.p4') \
                  $(shell find ${MODULE_DIR}/ -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*')
MODULE_POSTGEN_MK = module_rxdma_p4pd.mk  module_rxdma_swig.mk
include ${MKDEFS}/post.mk
