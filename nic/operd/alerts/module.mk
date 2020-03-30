# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = liboperd_alerts.lib
MODULE_PIPELINE = iris apollo apulu artemis gft athena
MODULE_INCS     = ${BLD_GEN_DIR}
MODULE_ARLIBS   =
MODULE_SOLIBS   = 
MODULE_LDLIBS   =
MODULE_FLAGS    = -Werror
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc)
include ${MKDEFS}/post.mk
