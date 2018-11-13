# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = upgrade.bin
MODULE_SOLIBS   = upgrade upgradeutils delphisdk sysmgr upgradeproto
MODULE_ARLIBS   = delphishm
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cc)
MODULE_SRCS     = $(filter-out , $(ALL_CC_FILES))

include ${MKDEFS}/post.mk
