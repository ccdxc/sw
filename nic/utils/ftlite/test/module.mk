# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

# TODO , Need to remove the pipeline dependent code. Un-used now
include ${MKDEFS}/pre.mk
MODULE_TARGET   = ftlite_test.gtest
MODULE_FLAGS    = -O0
MODULE_PIPELINE = artemis
MODULE_ARCH		= x86_64
MODULE_SOLIBS   = ftlite ftlitep4pd_mock ftlite_p4psim logger sdkxcvrdriver \
                  sdkp4 sdkp4utils sdkasicpd bm_allocator sdkpal sdkcapri \
                  ${NIC_SDK_SOLIBS} ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                  ${NIC_SDK_SOLIBS} \
                  ${NIC_HAL_PD_SOLIBS_${ARCH}}
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} dl
include ${MKDEFS}/post.mk
