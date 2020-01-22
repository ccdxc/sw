# # {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = pds_ms_hals_test_mockapi.gtest
MODULE_PREREQS  = metaswitch.submake
MODULE_PIPELINE = apulu
MODULE_ARCH     = x86_64
MODULE_INCS     = ${BLD_PROTOGEN_DIR} $(TOPDIR)/nic/metaswitch/stubs/hals \
                  $(addprefix $(MS_ROOT)/,$(MS_INCLPATH))
MODULE_FLAGS    = $(addprefix -D,$(MS_COMPILATION_SWITCH))
MODULE_SOLIBS   = shmmgr pdsmscommon pdsmshals pdstestapiutils \
                  pdsapimock pdsmsmgmt pdsmsstubs ipsfeeder model_client \
                  thread logger slab rte_indexer
MODULE_DEFS     = -DPDS_MOCKAPI
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
MODULE_LDLIBS   = rt dl $(MS_LD_LIBS)
include ${MKDEFS}/post.mk
