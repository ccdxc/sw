# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = pdsagent.bin
MODULE_PREREQS  = metaswitch.submake
MODULE_PIPELINE = apollo artemis apulu
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_SOLIBS   = pdsproto thread trace logger svc pdsapi memhash sltcam \
                  rfc_${PIPELINE} event_thread pdsrfc pdsagentcore slhash \
                  pdsmshals pdsmsmgmt pdsmscommon pdsmsstubs pdsmsmgmtsvc \
                  pdsfte ${NIC_${PIPELINE}_NICMGR_LIBS} ${NIC_FTL_LIBS} \
                  sdkeventmgr
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${NIC_COMMON_LDLIBS} edit ncurses $(MS_LD_LIBS)
include ${MKDEFS}/post.mk
