# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblinkmgrproto.lib
MODULE_PIPELINE = iris gft
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${BLD_PROTOGEN_DIR}/proto \
                  ${TOPDIR}/hal/third-party/grpc/include
MODULE_FLAGS    = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS  = linkmgr.proto penctl.proto libpenctlproto.lib
MODULE_SRCS     = ${BLD_PROTOGEN_DIR}/linkmgr/mac_metrics.pb.cc \
                  ${BLD_PROTOGEN_DIR}/linkmgr/mac_metrics.delphi.cc
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
MODULE_SOLIBS   = delphisdk penctlproto
include ${MKDEFS}/post.mk
