# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := test_mnet.bin
MODULE_INCS     := ${MODULE_SRC_DIR}/../include/
MODULE_SOLIBS   := mnet
include ${MKDEFS}/post.mk
