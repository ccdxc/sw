# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = delphi_server_test.gtest
MODULE_PIPELINE = iris
MODULE_INCS     = ${BLD_GEN_DIR}/proto
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = delphisdk halproto upgradeproto nicmgrproto commonproto
MODULE_ARLIBS   = delphimessenger delphishm
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev dl
MODULE_SRCS     = ${MODULE_SRC_DIR}/delphi_server_test.cc ${MODULE_SRC_DIR}/delphi_server.cc
include ${MKDEFS}/post.mk
