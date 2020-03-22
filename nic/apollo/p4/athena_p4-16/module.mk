# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
ifeq "${P4VER}" "P4_16"
MODULE_TARGET   = athena_p4.p4bin
MODULE_SRCS     = ${MODULE_SRC_DIR}/athena.p4
MODULE_PIPELINE = athena
MODULE__P4VER   = P4_16
MODULE_P4C      = P4_16
MODULE_P4C_OPTS = --out-dir ${BLD_P4GEN_DIR} \
                  --emit-api --log-level debug --use-deparse-graph \
                  --cfg-dir ${BLD_PGMBIN_DIR}/athena_p4 --prog-name athena
MODULE_NCC_OPTS = --pipeline=athena --asm-out --gen-dir ${BLD_P4GEN_DIR} \
                  --cfg-dir ${BLD_PGMBIN_DIR}/athena_p4 \
                  --i2e-user --fe-flags="-I${TOPDIR} -I${SDKDIR}"
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h')
MODULE_POSTGEN_MK = module_p4pd.mk
MODULE_BIN_DIR    = ${BLD_BIN_DIR}/p4asm
else
MODULE_PIPELINE   = athena_dontuse
MODULE_TARGET     = athena_p4.dontuse
endif
include ${MKDEFS}/post.mk
