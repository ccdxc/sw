# {C} Copyright 2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = session_age.p4bin
MODULE_PIPELINE = athena
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi \
                  --two-byte-profile --fe-flags="-I${TOPDIR} -I${SDKDIR} -I${SDKDIR}" \
                                  --gen-dir ${BLD_P4GEN_DIR}
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h') \
                  $(shell find ${MODULE_DIR}/../include -name '*') \
                                  $(shell find ${MODULE_DIR}/../common-p4+ -name '*.p4')
include ${MKDEFS}/post.mk
