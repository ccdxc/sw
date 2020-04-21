# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsagent_athena.lib
MODULE_PIPELINE = athena
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_SOLIBS   = svc_athena pdsproto pdsagentcore_athena sdkeventmgr
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
include ${MKDEFS}/post.mk
