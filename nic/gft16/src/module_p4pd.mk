# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libp4pd_gft16.lib
MODULE_PIPELINE = gft16
MODULE_SRC_DIR  = ${BLD_P4GEN_DIR}/gft16/src/
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*p4pd.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*table_range.cc)
include ${MKDEFS}/post.mk
