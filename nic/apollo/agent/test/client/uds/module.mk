# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = libudsagentclient.lib
MODULE_PIPELINE     = apulu
MODULE_INCS         = ${BLD_PROTOGEN_DIR}
MODULE_LDLIBS       = ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_SOLIBS       = trace logger
MODULE_SRCS         = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                      $(wildcard ${BLD_PROTOGEN_DIR}/*.cc) \
                      $(wildcard ${BLD_PROTOGEN_DIR}/meta/*.cc) \
                      ${TOPDIR}/nic/apollo/agent/trace.cc
include ${MKDEFS}/post.mk
