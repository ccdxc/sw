# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET        = testapp.bin
MODULE_PIPELINE      = apollo artemis apulu
MODULE_INCS          = /usr/local/include \
                       ${TOPDIR}/nic/hal/third-party/google/include \
                       ${BLD_PROTOGEN_DIR}
MODULE_LDLIBS        = pthread \
                       ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                       ${NIC_COMMON_LDLIBS}
MODULE_SOLIBS        = pdsproto trace logger agentclient
MODULE_FLAGS         = -DTEST_GRPC_APP
MODULE_EXCLUDE_FLAGS = -O2
MODULE_SRCS          = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                       ${TOPDIR}/nic/apollo/test/scale/test.cc \
                       ${TOPDIR}/nic/apollo/agent/trace.cc \
                       ${TOPDIR}/nic/apollo/test/scale/api_grpc.cc \
                       $(wildcard ${BLD_PROTOGEN_DIR}/*.cc) \
                       $(wildcard ${BLD_PROTOGEN_DIR}/meta/*.cc) \
                       $(wildcard ${BLD_PROTOGEN_DIR}/types/*.cc)
include ${MKDEFS}/post.mk
