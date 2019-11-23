# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = pegasus
MODULE_PREREQS  = metaswitch.submake
MODULE_PIPELINE = apulu
MODULE_ARCH     = x86_64
MODULE_INCS 	= $(addprefix $(MS_ROOT)/,$(MS_INCLPATH))
MODULE_FLAGS	= $(addprefix -D,$(MS_COMPILATION_SWITCH))
MODULE_SOLIBS   = pdsamgmt pdsamgmtsvc pdsahals_mock
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
MODULE_LDLIBS   = $(MS_LD_LIBS)
include ${MKDEFS}/post.mk
