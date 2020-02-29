# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = sdk_metrics_test.gtest
MODULE_PIPELINE = iris apollo apulu artemis
MODULE_SOLIBS   = pal penmetrics sdkpal logger
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev pthread z dl
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
