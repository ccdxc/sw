# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = slhash_test.gtest
MODULE_SOLIBS   = slhash slhash_p4pd_mock logger indexer
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
