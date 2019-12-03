# # {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = pdsa_sim_mockapi.gtest
MODULE_PREREQS  = metaswitch.submake
MODULE_PIPELINE = apulu
MODULE_ARCH     = x86_64
MODULE_INCS     = $(TOPDIR)/nic/metaswitch/stubs/hals \
                  $(addprefix $(MS_ROOT)/,$(MS_INCLPATH))  ${MODULE_GEN_DIR}
MODULE_FLAGS    = $(addprefix -D,$(MS_COMPILATION_SWITCH))
MODULE_SOLIBS   = shmmgr pdsacommon pdsahals pdstestutils pdsamgmt pdsamgmtsvc \
                  pdsapimock pdsastubs model_client thread logger pdsgenproto
MODULE_DEFS     = -DPDS_MOCKAPI
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
MODULE_LDLIBS   = rt dl $(MS_LD_LIBS) ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${NIC_COMMON_LDLIBS} edit ncurses
include ${MKDEFS}/post.mk
