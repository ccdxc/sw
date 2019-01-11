# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = capsim_libs.export
MODULE_EXPORT_DIR   = ${TOPDIR}/nic/asic/capri/model/capsim-master/gen/${ARCH}
MODULE_PIPELINE     = iris gft apollo
include ${MKDEFS}/post.mk
