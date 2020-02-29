# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = penmetrics_slab_test.gtest
MODULE_PIPELINE = iris apollo apulu gft artemis
MODULE_ARCH     = x86_64
MODULE_SOLIBS   = penmetrics
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev dl
MODULE_SRCS     = ${MODULE_SRC_DIR}/slab_mgr_test.cc
include ${MKDEFS}/post.mk

