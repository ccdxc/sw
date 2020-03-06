# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libp4pd_apulu.lib
MODULE_PREREQS  = apulu_p4.p4bin
MODULE_PIPELINE = apulu
MODULE_SRC_DIR  = ${BLD_P4GEN_DIR}/apulu/src/
MODULE_SOLIBS   = utils
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*p4pd.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*table_range.cc)
MODULE_FLAGS    = -O3
include ${MKDEFS}/post.mk
