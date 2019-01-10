# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = hal.svcgen
MODULE_PIPELINE     = iris gft
MODULE_PREREQS      = hal.proto
MODULE_DEPS         = $(wildcard ${MODULE_SRC_DIR}/*.proto) \
                      $(wildcard ${TOPDIR}/nic/tools/hal/*)
MODULE_POSTGEN_MK   = module_libsvcgen.mk
include ${MKDEFS}/post.mk
