# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libcommonproto.lib
MODULE_PIPELINE = iris gft
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${TOPDIR}/hal/third-party/grpc/include \
                  ${TOPDIR}/nic/delphi/proto/delphi \
MODULE_FLAGS    = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS  = common.proto
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
MODULE_SOLIBS   = delphisdk
MODULE_SRCS     = ${BLD_PROTOGEN_DIR}/common/nicmgr_status_msgs.pb.cc \
                  ${BLD_PROTOGEN_DIR}/common/nicmgr_status_msgs.delphi.cc 
include ${MKDEFS}/post.mk
