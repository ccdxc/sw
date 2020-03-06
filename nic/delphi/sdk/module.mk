# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libdelphisdk.lib
MODULE_PIPELINE = iris gft
MODULE_PREREQS  = libev.export
MODULE_ARLIBS   = delphimessenger delphishm delphiutils delphiclientproto  delphiproto messengerproto
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cc)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cc)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
include ${MKDEFS}/post.mk
