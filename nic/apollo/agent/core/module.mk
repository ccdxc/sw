# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsagentcore.so
MODULE_PIPELINE = apollo artemis
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_SOLIBS   = pdsapi
include ${MKDEFS}/post.mk
