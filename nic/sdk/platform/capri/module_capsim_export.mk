# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = capsim_libs.export
MODULE_EXPORT_DIR   = ${SDKDIR}/third-party/asic/capri/model/capsim-master/gen/${ARCH}
MODULE_ASIC         = capri
include ${MKDEFS}/post.mk
