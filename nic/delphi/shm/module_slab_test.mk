# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = delphi_slab_test.gtest
MODULE_ARLIBS   = delphishm delphiutils
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev
MODULE_SRCS     = ${MODULE_SRC_DIR}/slab_mgr_test.cc

include ${MKDEFS}/post.mk
