# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = hal.bin
MODULE_PIPELINE = iris gft
MODULE_SRCS     := ${MODULE_SRC_DIR}/main.cc
ifeq ($(PIPELINE),gft)
MODULE_SRCS     := ${MODULE_SRCS} \
                   ${MODULE_SRC_DIR}/svc_gft.cc
else
MODULE_SRCS     := ${MODULE_SRCS} \
                   ${MODULE_SRC_DIR}/svc_iris.cc
endif
MODULE_SOLIBS   = ${NIC_HAL_ALL_SOLIBS} delphisdk packet_parser
MODULE_LDLIBS   = ${NIC_HAL_ALL_LDLIBS} ev ${SDK_THIRDPARTY_CAPRI_LDLIBS}
include ${MKDEFS}/post.mk
