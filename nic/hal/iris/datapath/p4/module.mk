# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = iris.p4bin
MODULE_SRCS         = ${MODULE_SRC_DIR}/p4.p4
MODULE_PIPELINE     = iris
MODULE_NCC_OPTS     = --asm-out --pd-gen --gen-dir ${BLD_P4GEN_DIR} \
                      --cfg-dir ${BLD_PGMBIN_DIR} \
                      --split-deparse-only-headers \
                      --pipeline ${PIPELINE}
ifeq ($(PLATFORM),haps)
MODULE_NCC_OPTS     += --fe-flags="-DPLATFORM_HAPS -I${TOPDIR}"
else
MODULE_NCC_OPTS     += --fe-flags="-I${TOPDIR}"
endif

MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h')
MODULE_POSTGEN_MK   = module_p4pd.mk
include ${MKDEFS}/post.mk
