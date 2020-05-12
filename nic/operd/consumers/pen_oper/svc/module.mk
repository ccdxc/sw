# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpen_oper_svc.lib
MODULE_PIPELINE = apulu
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_SOLIBS   = operd operd_alerts operd_alert_defs pen_oper_core penmetrics
MODULE_PREREQS  = operdgen.proto
include ${MKDEFS}/post.mk
