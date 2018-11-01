# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = delphi_server_test.gtest
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = delphisdk
MODULE_ARLIBS   = delphimessenger
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev
MODULE_SRCS     = ${MODULE_SRC_DIR}/delphi_server_test.cc ${MODULE_SRC_DIR}/delphi_server.cc

include ${MKDEFS}/post.mk
