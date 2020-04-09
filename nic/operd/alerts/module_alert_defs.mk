# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = liboperd_alert_defs.lib
MODULE_PIPELINE = iris apollo apulu artemis gft athena
MODULE_INCS     = 
MODULE_ARLIBS   = 
MODULE_SOLIBS   = 
MODULE_LDLIBS   =
MODULE_DEPS     = 
MODULE_FLAGS    = -Werror
MODULE_SRC_DIR  = ${BLD_GEN_DIR}/alerts
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc)
MODULE_PREREQS  = liboperd_alerts.submake
include ${MKDEFS}/post.mk
