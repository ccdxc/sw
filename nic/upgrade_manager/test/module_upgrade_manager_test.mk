# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = upgrade_manager_test.gtest
MODULE_SOLIBS   = delphisdk upgrade sysmgr upgradeproto
MODULE_ARLIBS   = delphishm
MODULE_ARCH     = x86_64
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev pthread
MODULE_SRCS     = ${MODULE_SRC_DIR}/upgrade_manager_test.cc

include ${MKDEFS}/post.mk
