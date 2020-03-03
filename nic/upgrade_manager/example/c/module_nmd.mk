# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = examplenmd.bin
MODULE_PIPELINE = iris
MODULE_SOLIBS   = delphisdk utils upgrade_app upgradeproto sdkpal logger sysmgr upgradeutils
MODULE_ARLIBS   = delphishm
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev dl
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*nmd*.cc)
#MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
MODULE_SRCS     = $(ALL_CC_FILES)
include ${MKDEFS}/post.mk
