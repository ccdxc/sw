# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = operd.bin
MODULE_PIPELINE = iris apollo apulu artemis gft
MODULE_SOLIBS   = operd
MODULE_LDLIBS   = rt pthread dl
MODULE_ARLIBS   =
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cc)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cc)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
ifneq ($(PIPELINE), apulu)
MODULE_SRCS     += $(wildcard ${MODULE_SRC_DIR}/impl/stub/*.cc)
else
MODULE_SRCS     += $(wildcard ${MODULE_SRC_DIR}/impl/${PIPELINE}/*.cc)
MODULE_SOLIBS   += operdproto operdsvc event_thread thread logger utils \
                   sdkeventmgr list slab shmmgr ht sdkpal
MODULE_LDLIBS   += ${NIC_THIRDPARTY_GOOGLE_LDLIBS} ${NIC_COMMON_LDLIBS}
MODULE_INCS     = ${MODULE_GEN_DIR}
endif
include ${MKDEFS}/post.mk
