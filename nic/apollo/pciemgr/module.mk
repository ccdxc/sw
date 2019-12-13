# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdspciemgr.so
MODULE_PIPELINE = apollo artemis apulu athena
MODULE_INCS     := ${MODULE_SRC_DIR}
include ${MKDEFS}/post.mk
