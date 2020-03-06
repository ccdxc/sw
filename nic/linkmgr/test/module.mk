# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = port_client.bin
MODULE_PIPELINE = iris gft
MODULE_SRCS   = ${MODULE_SRC_DIR}/port_client.cc
MODULE_SOLIBS = halproto
#linkmgr_test_utils bm_allocator \
                hal_lib agent_api linkmgr_src block_list \
                list shmmgr slab ht timerfd periodic halproto \
                linkmgr_src sdklinkmgr \
                linkmgr_libsrc linkmgrcsr linkmgrcsr_helpers \
                linkmgr_capricsr_int \
                logger thread twheel sdkpal catalog utils mtrack trace
MODULE_LDLIBS = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                ${NIC_COMMON_LDLIBS}
#${NIC_HAL_ALL_LDLIBS}${SDK_THIRDPARTY_CAPRI_LDLIBS}
MODULE_FLAGS  = -pthread -rdynamic
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
include ${MKDEFS}/post.mk
