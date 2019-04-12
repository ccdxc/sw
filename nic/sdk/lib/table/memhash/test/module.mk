# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

# TODO , Need to remove the pipeline dependent code. Un-used now
include ${MKDEFS}/pre.mk
MODULE_TARGET   = memhash_test.gtest
MODULE_SOLIBS   = memhash memhashp4pd_mock logger
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_FLAGS    = -O3
MODULE_ARCH   = x86_64
include ${MKDEFS}/post.mk
