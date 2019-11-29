# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
#
include ${MKDEFS}/pre.mk
MODULE_TARGET   = test_mgmt_grpc.bin
MODULE_PIPELINE = apulu
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/test_mgmt.cc)
MODULE_SOLIBS   = pdsgenproto pdsproto
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} z
MODULE_FLAGS    =  -O2
include ${MKDEFS}/post.mk
