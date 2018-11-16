# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     :=
MODULE_TARGET   := hal_api_filter_smart_test.gtest
MODULE_SOLIBS   := hal_api halproto indexer logger
MODULE_LDLIBS   := ev ${NIC_COMMON_LDLIBS} ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_INCS     := ${BLD_PROTOGEN_DIR}/
include ${MKDEFS}/post.mk
