# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = ipfix.asmbin
MODULE_PREREQS      = ipfix.p4bin iris.p4bin
MODULE_PIPELINE     = iris
MODULE_INCS         = ${BLD_GEN_DIR} \
                      ${TOPDIR}/nic/include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
