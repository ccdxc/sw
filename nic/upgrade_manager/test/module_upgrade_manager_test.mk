# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = upgrade_manager_test.gtest
MODULE_SOLIBS   = delphisdk upgrade
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev
MODULE_ARLIBS   = upgradeproto
MODULE_SRCS     = ${MODULE_SRC_DIR}/upgrade_manager_test.cc

include ${MKDEFS}/post.mk
