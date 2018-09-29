# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libp4pd_iris.so
MODULE_PREREQS  = iris.p4bin
MODULE_PIPELINE = iris
MODULE_SRC_DIR  = ${BLD_P4GEN_DIR}/p4/src/
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*p4pd.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*table_range.cc)
include ${MKDEFS}/post.mk
