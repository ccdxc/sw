# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = linkmgr_delphi_test.gtest
MODULE_ARCH     = x86_64
MODULE_PIPELINE = iris gft
MODULE_SRCS   = ${MODULE_SRC_DIR}/linkmgr_delphi_test.cc
MODULE_SOLIBS = pal sdkfru linkmgr_test_utils bm_allocator linkmgrdelphi \
                hal_lib agent_api block_list \
                list shmmgr slab ht timerfd periodic halproto delphisdk \
                linkmgr_src sdklinkmgr haltrace \
                linkmgr_libsrc sdklinkmgrcsr \
                sdkcapri_csrint \
                logger thread twheel sdkpal catalog utils mtrack trace
MODULE_LDLIBS = ${NIC_HAL_ALL_LDLIBS}
MODULE_FLAGS  = -pthread -rdynamic
include ${MKDEFS}/post.mk
