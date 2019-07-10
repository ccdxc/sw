# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblinkmgr_test_utils.so
MODULE_PIPELINE = iris gft
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
include ${MKDEFS}/post.mk
