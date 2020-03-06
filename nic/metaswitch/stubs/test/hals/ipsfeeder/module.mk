# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_PREREQS  = metaswitch.submake
MODULE_TARGET   = libipsfeeder.lib
MODULE_PIPELINE = apollo artemis apulu
MODULE_ARCH     = x86_64
MODULE_INCS 	= $(TOPDIR)/nic/metaswitch/stubs/hals \
                  $(addprefix $(MS_ROOT)/,$(MS_INCLPATH))  ${MODULE_GEN_DIR}
MODULE_FLAGS    = $(addprefix -D,$(MS_COMPILATION_SWITCH))
MODULE_SOLIBS   = pdsgenproto pdsmsmgmt pdsmsmgmtsvc
MODULE_FLAGS	= $(addprefix -D,$(MS_COMPILATION_SWITCH))
include ${MKDEFS}/post.mk
