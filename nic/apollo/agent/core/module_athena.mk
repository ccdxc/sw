# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsagentcore_athena.lib
MODULE_PIPELINE = athena
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_SRCS     = $(wildcard ${MODULE_DIR}/state.cc) \
                  $(wildcard ${MODULE_DIR}/svc_thread.cc)
MODULE_SOLIBS   = pdsapi event_thread upgrade_ev
include ${MKDEFS}/post.mk
