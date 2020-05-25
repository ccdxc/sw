# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
ifeq "${P4VER}" "P4_14"
MODULE_TARGET       = iris.p4bin
MODULE_SRCS         = ${MODULE_SRC_DIR}/p4.p4
MODULE_PIPELINE     = iris
MODULE_NCC_OPTS     = --asm-out --pd-gen --gen-dir ${BLD_P4GEN_DIR} \
                      --cfg-dir ${BLD_PGMBIN_DIR} \
                      --split-deparse-only-headers \
                      --phv-flits=10 \
                      --pipeline ${PIPELINE}

ifeq ($(PLATFORM),haps)
ifeq ($(ASIC),elba)
MODULE_NCC_OPTS     += --asic elba --fe-flags="-DELBA -DPLATFORM_HAPS -I${TOPDIR} -I${SDKDIR}"
else
MODULE_NCC_OPTS     += --fe-flags="-DPLATFORM_HAPS -I${TOPDIR} -I${SDKDIR}"
endif
else
ifeq ($(ASIC),elba)
MODULE_NCC_OPTS     += --asic elba --fe-flags="-DELBA -I${TOPDIR} -I${SDKDIR}"
else
MODULE_NCC_OPTS     += --fe-flags="-I${TOPDIR} -I${SDKDIR}"
endif
endif

MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h')
MODULE_POSTGEN_MK   = module_p4pd.mk
else
MODULE_PIPELINE     = iris_dontuse
MODULE_TARGET       = iris.p4bin.dontuse
endif
include ${MKDEFS}/post.mk
