# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = adminq.p4bin
MODULE_PIPELINE = iris gft apollo artemis apulu athena
MODULE_SRCS     = ${MODULE_SRC_DIR}/adminq_txdma_actions.p4 \
                  ${MODULE_SRC_DIR}/nicmgr_txdma_actions.p4

ifeq ($(ASIC),elba)
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi --asic elba \
                  --two-byte-profile --fe-flags="-DELBA -I${TOPDIR} -I${SDKDIR} -I${SDKDIR}" \
                  --gen-dir ${BLD_P4GEN_DIR}
else
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi \
                  --two-byte-profile --fe-flags="-I${TOPDIR} -I${SDKDIR} -I${SDKDIR}" \
                  --gen-dir ${BLD_P4GEN_DIR}
endif

MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*') \
                  $(shell find ${MODULE_DIR}/../common-p4+ -name '*.p4')
include ${MKDEFS}/post.mk
