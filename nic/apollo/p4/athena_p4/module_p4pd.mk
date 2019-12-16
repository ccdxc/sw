# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libp4pd_athena.so
MODULE_PREREQS  = athena_p4.p4bin
MODULE_PIPELINE = athena
MODULE_SRC_DIR  = ${BLD_P4GEN_DIR}/athena/src/
MODULE_SOLIBS   = utils
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*p4pd.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*table_range.cc)
MODULE_FLAGS    = -O3
include ${MKDEFS}/post.mk
