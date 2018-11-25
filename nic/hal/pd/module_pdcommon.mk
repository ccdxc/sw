# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdcommon.so
#MODULE_PIPELINE = iris gft
MODULE_SRCS    = ${MODULE_SRC_DIR}/asic_pd.cc \
                 ${MODULE_SRC_DIR}/hal_pd.cc
include ${MKDEFS}/post.mk
