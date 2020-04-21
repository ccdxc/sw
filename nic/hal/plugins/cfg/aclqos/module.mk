# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libcfg_plugin_aclqos.lib
MODULE_PREREQS  = hal.svcgen
MODULE_PIPELINE = iris gft
ifeq ($(ASIC),elba)
MODULE_FLAGS    += -DELBA
endif
include ${MKDEFS}/post.mk
