# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = pdsa_store_test.gtest
MODULE_PREREQS  = metaswitch.submake
MODULE_PIPELINE = apulu
MODULE_ARCH     = x86_64
MODULE_INCS 	= $(TOPDIR)/nic/metaswitch/stubs/hals \
		  $(addprefix $(MS_ROOT)/,$(MS_INCLPATH))
MODULE_FLAGS	= $(addprefix -D,$(MS_COMPILATION_SWITCH))
MODULE_SOLIBS   = shmmgr pdsacommon
MODULE_LDLIBS   = rt
include ${MKDEFS}/post.mk
