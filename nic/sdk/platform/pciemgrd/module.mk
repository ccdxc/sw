# {C} Copyright 2018-2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   := libpciemgrd.lib
MODULE_INCS     := ${BLD_PROTOGEN_DIR}
MODULE_SOLIBS   := penipc_ev upgrade_ev penipc utils
include ${MKDEFS}/post.mk
