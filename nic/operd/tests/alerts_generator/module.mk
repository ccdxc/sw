# {C} Copyright 2020 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = alerts_gen.bin
MODULE_PIPELINE = apulu
MODULE_SOLIBS   = operd operd_alert_defs operd_alerts 
MODULE_LDLIBS   = rt
include ${MKDEFS}/post.mk
