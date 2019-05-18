# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

# TODO , Need to remove the pipeline dependent code. Un-used now
include ${MKDEFS}/pre.mk
MODULE_TARGET   = ftlv4_test.gtest
MODULE_SOLIBS   = ftlv4 ftlp4pd_mock_ipv4 logger sdkpal sdkcapri \
                  sdkp4 sdkp4utils sdkasicpd bm_allocator \
                  ${NIC_SDK_SOLIBS} ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} \
                  ${NIC_HAL_PD_SOLIBS_${ARCH}}
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} dl
MODULE_FLAGS    = -O3 ${NIC_CSR_FLAGS}
MODULE_ARCH     = x86_64
MODULE_PIPELINE = apollo
MODULE_INCS     = ${NIC_CSR_INCS}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
include ${MKDEFS}/post.mk
