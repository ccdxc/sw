# {C} Copyright 2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libcfg_plugin_ncsi.lib
MODULE_PIPELINE = iris gft
MODULE_INCS    = ${BLD_PROTOGEN_DIR}/
include ${MKDEFS}/post.mk
