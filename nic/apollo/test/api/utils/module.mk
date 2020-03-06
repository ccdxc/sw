# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET      = libpdstestapiutils.lib
MODULE_PIPELINE    = apollo artemis apulu athena
MODULE_SRCS        = $(wildcard ${MODULE_SRC_DIR}/*.cc)
ifdef AGENT_MODE
    MODULE_SRCS   += ${TOPDIR}/nic/apollo/test/scale/api_grpc.cc
    MODULE_SOLIBS += agentclient
    MODULE_INCS    = ${BLD_PROTOGEN_DIR}
    MODULE_LDLIBS  = ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
    MODULE_FLAGS   = -DAGENT_MODE
else
    MODULE_SRCS   += ${TOPDIR}/nic/apollo/test/scale/api_pds.cc
    MODULE_FLAGS   = -DSKIP_FLOW
endif
include ${MKDEFS}/post.mk
