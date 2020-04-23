# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = athena_app.bin
MODULE_PIPELINE = athena
MODULE_INCS     = ${MODULE_GEN_DIR} ${SDKDIR}/dpdk/build/include/
MODULE_LDFLAGS  = -L$(MS_LIB_DIR)
MODULE_SOLIBS   = pal pdsframework pdscore pdslearn pdsapi pdsapi_impl \
                  thread trace logger pdsapi memhash sltcam \
                  pdsagent_athena\
                  event_thread slhash penmetrics \
                  ${NIC_FTL_LIBS} \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  sdkp4 sdkp4utils sdk_asicrw_if sdk${ASIC} pdsapi_asic_impl \
                  sdkplatformutils sdkxcvrdriver sdkasicpd sdkeventmgr kvstore_lmdb \
                  bm_allocator sdklinkmgr sdklinkmgrcsr memhash rte_indexer \
                  sltcam slhash ${NIC_${PIPELINE}_NICMGR_LIBS}
MODULE_FLAGS    = -O3
ifeq ($(ARCH), x86_64)
MODULE_FLAGS  += -march=native
endif
MODULE_DEFS     = -DRTE_FORCE_INTRINSICS
MODULE_PREREQS  = dpdk.submake
MODULE_LDLIBS   =  ${NIC_COMMON_LDLIBS} \
                   ${NIC_CAPSIM_LDLIBS} ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                   AAPL edit ncurses dpdk lmdb zmq
ifeq ($(ARCH), x86_64)
MODULE_INCS     +=  ${CONFIG_GTEST_INCS}
MODULE_LIBS     +=  ${CONFIG_GTEST_LIBS}
MODULE_EXCLUDE_FLAGS += ${CONFIG_GTEST_EXCLUDE_FLAGS}
MODULE_FLAGS    +=  ${CONFIG_GTEST_FLAGS}
MODULE_LDPATHS  += -L${COMMON_GTEST_LDPATHS}
endif

include ${MKDEFS}/post.mk
