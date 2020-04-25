# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsvc_athena.lib
MODULE_PIPELINE = athena
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_SOLIBS   = pal pdsframework pdscore pdsapi pdsapi_impl \
                  ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}} \
                  sdkp4 sdkp4utils sdk_asicrw_if sdk${ASIC} \
                  sdkplatformutils sdkxcvrdriver sdkasicpd \
                  bm_allocator sdklinkmgr sdklinkmgrcsr operd \
                  operd_alerts
MODULE_SRCS     = $(wildcard ${MODULE_DIR}/port.cc) \
                  $(wildcard ${MODULE_DIR}/device.cc) \
                  $(wildcard ${MODULE_DIR}/svc_thread.cc) \
                  $(wildcard ${MODULE_DIR}/pds_ms_svc_stub_athena.cc) \
                  $(wildcard ${MODULE_DIR}/svc_stub_athena.cc) \
                  $(wildcard ${MODULE_DIR}/debug.cc)
MODULE_LDLIBS   = ${NIC_CAPSIM_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  AAPL
include ${MKDEFS}/post.mk
