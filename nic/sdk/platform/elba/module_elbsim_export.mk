# {C} Copyright 2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = elbsim_libs.export
MODULE_EXPORT_DIR   = ${SDKDIR}/third-party/asic/ip/verif/pensim/gen/${ARCH}
#MODULE_PIPELINE     = iris gft apollo elektra
#MODULE_ASIC         = elba
include ${MKDEFS}/post.mk
