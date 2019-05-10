# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libplugin_sfw.so
MODULE_PIPELINE = iris gft
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/cfg/*.cc)
MODULE_INCS    = ${BLD_PROTOGEN_DIR}/
include ${MKDEFS}/post.mk
