# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = delphi_example_test.gtest
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = delphisdk sdkpal logger
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev dl
MODULE_ARLIBS   = delphiexampleproto delphishm
MODULE_SRCS     = ${MODULE_SRC_DIR}/example_test.cc ${MODULE_SRC_DIR}/example.cc \
                  ${MODULE_SRC_DIR}/example_stats.cc

include ${MKDEFS}/post.mk
