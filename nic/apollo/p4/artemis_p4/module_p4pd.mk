# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libp4pd_artemis.lib
MODULE_PREREQS  = artemis_p4.p4bin
MODULE_PIPELINE = artemis
MODULE_SRC_DIR  = ${BLD_P4GEN_DIR}/artemis/src/
MODULE_SOLIBS   = utils
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*p4pd.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/*table_range.cc)
MODULE_FLAGS    = -O3
include ${MKDEFS}/post.mk
