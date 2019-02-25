# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = halapollo.bin
MODULE_PIPELINE = apollo
MODULE_SRCS     = ${MODULE_SRC_DIR}/main.cc
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_SOLIBS   = tpcproto halapollosvc
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${NIC_COMMON_LDLIBS}
include ${MKDEFS}/post.mk
