# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libupgrade_app.lib
MODULE_PREREQS  = upg.submake
MODULE_PIPELINE = iris gft
MODULE_SOLIBS   = delphisdk upgradeutils
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cc ${MODULE_SRC_DIR}/../../include/c/*cc)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
include ${MKDEFS}/post.mk
