# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

# TODO , Need to remove the pipeline dependent code. Un-used now
include ${MKDEFS}/pre.mk

MODULE_TARGET   = ${PIPELINE}_ftl_test.gtest
MODULE_SOLIBS_COMMON = ${NIC_FTL_LIBS} logger sdkpal sdkcapri \
                       sdkp4 sdkp4utils sdkasicpd bm_allocator sdkxcvrdriver\
                       ${NIC_SDK_SOLIBS} ${NIC_${PIPELINE}_P4PD_SOLIBS} \
                       ${NIC_SDK_SOLIBS} ${NIC_HAL_PD_SOLIBS_${ARCH}}
MODULE_SOLIBS_iris  = hal_mock hal_lib haltrace halproto
MODULE_SOLIBS   = ${MODULE_SOLIBS_COMMON} ${MODULE_SOLIBS_${PIPELINE}}

MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                  ${SDK_THIRDPARTY_CAPRI_LDLIBS} \
                  ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS} dl ssl crypto

MODULE_SRCS  = $(wildcard ${MODULE_DIR}/*.cc) \
               $(wildcard ${MODULE_DIR}/${PIPELINE}/*.cc) \
               $(wildcard ${MODULE_DIR}/${PIPELINE}/p4pd_mock/*.cc)
MODULE_FLAGS    = -O3 ${NIC_CSR_FLAGS}
MODULE_ARCH     = x86_64
MODULE_PIPELINE = iris apulu
MODULE_INCS     = ${NIC_CSR_INCS} utils/ftl/test/${PIPELINE}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
include ${MKDEFS}/post.mk
