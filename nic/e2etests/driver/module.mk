# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libe2e_driver.lib
MODULE_ARCH     = x86_64
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/lib_driver.cc
MODULE_INCS     = ${THIRD_PARTY_INCLUDES}
include ${MKDEFS}/post.mk
