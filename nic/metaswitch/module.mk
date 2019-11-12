# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = pdsa
MODULE_PREREQS  = metaswitch.submake
MODULE_PIPELINE = apulu
MODULE_ARCH	= x86_64
MODULE_INCS 	= $(addprefix $(MS_ROOT)/,$(MS_INCLPATH))
MODULE_FLAGS	= $(addprefix -D,$(MS_COMPILATION_SWITCH))
MODULE_SOLIBS   = pdsahals pdsamgmt pdsacommon pal pdsframework pdscore pdsapi pdsapi_impl \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  sdkp4 sdkp4utils sdkcapri_asicrw_if sdkcapri \
                  sdkplatformutils sdkxcvrdriver sdkasicpd \
                  lpmitree_${PIPELINE} rfc_${PIPELINE} pdsrfc \
                  bm_allocator sdklinkmgr sdklinkmgrcsr \
                  memhash sltcam slhash ${NIC_${PIPELINE}_NICMGR_LIBS}
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
MODULE_LDLIBS   = ${NIC_COMMON_LDLIBS} \
                  ${NIC_CAPSIM_LDLIBS} \
                  ${${PIPELINE}_GTEST_COMMON_LDLIBS} \
                  $(MS_LD_LIBS) AAPL
include ${MKDEFS}/post.mk
