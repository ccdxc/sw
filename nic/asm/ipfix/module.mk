# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = ipfix.asmbin
MODULE_PREREQS      = ipfix.p4bin iris.p4bin
MODULE_PIPELINE     = iris
MODULE_INCS         = ${BLD_P4GEN_DIR} \
                      ${TOPDIR}/nic/include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
