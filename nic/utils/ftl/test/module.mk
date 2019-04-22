# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

# TODO , Need to remove the pipeline dependent code. Un-used now
include ${MKDEFS}/pre.mk
MODULE_TARGET   = ftl_test.gtest
MODULE_SOLIBS   = ftl ftlp4pd_mock logger sdkpal
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} dl
MODULE_FLAGS    = -O3
MODULE_ARCH     = x86_64
MODULE_PIPELINE = apollo
include ${MKDEFS}/post.mk
