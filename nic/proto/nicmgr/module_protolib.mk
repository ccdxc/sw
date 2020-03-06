# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libnicmgrproto.lib
MODULE_PIPELINE = iris gft
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${BLD_PROTOGEN_DIR}/proto \
                  ${TOPDIR}/hal/third-party/grpc/include
MODULE_FLAGS    = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS  = nicmgr.proto penctl.proto libpenctlproto.lib
MODULE_SRCS     = ${BLD_PROTOGEN_DIR}/nicmgr/nicmgr.pb.cc \
                  ${BLD_PROTOGEN_DIR}/nicmgr/nicmgr.delphi.cc \
                  ${BLD_PROTOGEN_DIR}/nicmgr/metrics.pb.cc \
                  ${BLD_PROTOGEN_DIR}/nicmgr/metrics.delphi.cc \
                  ${BLD_PROTOGEN_DIR}/nicmgr/accel_metrics.pb.cc \
                  ${BLD_PROTOGEN_DIR}/nicmgr/accel_metrics.delphi.cc
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
MODULE_SOLIBS   = delphisdk penctlproto
include ${MKDEFS}/post.mk
