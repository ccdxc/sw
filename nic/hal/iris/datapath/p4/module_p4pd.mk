# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
ifeq "${P4VER}" "P4_14"
MODULE_TARGET   = libp4pd_iris.lib
MODULE_PREREQS  = iris.p4bin
MODULE_PIPELINE = iris
MODULE_SRC_DIR  = ${BLD_P4GEN_DIR}/p4/src/
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*p4pd.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*table_range.cc)
MODULE_FLAGS    = -O3
else
MODULE_PIPELINE = iris_dontuse
MODULE_TARGET   = libp4pd_iris.lib.dontuse
endif
include ${MKDEFS}/post.mk
