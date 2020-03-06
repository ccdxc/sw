# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhal_svc_gen.lib
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = hal.svcgen
MODULE_SRC_DIR  = ${BLD_SVCGEN_DIR}
include ${MKDEFS}/post.mk
