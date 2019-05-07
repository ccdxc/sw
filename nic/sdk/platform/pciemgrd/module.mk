# {C} Copyright 2018-2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := libpciemgrd.so
MODULE_PIPELINE := apollo iris
MODULE_INCS     := ${BLD_PROTOGEN_DIR}
include ${MKDEFS}/post.mk
