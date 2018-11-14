# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk

MODULE_TARGET   = exampleupgapp.bin
MODULE_SOLIBS   = delphisdk utils upgrade_app upgradeproto
MODULE_ARLIBS   = delphishm
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt ev
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*example*.cc)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))

include ${MKDEFS}/post.mk
