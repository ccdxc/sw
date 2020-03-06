# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libnaplesstatusproto.lib
MODULE_PIPELINE = iris
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${TOPDIR}/hal/third-party/grpc/include
MODULE_FLAGS    = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS  = naples_status.proto
MODULE_ARLIBS   = delphishm
MODULE_SRCS     = ${BLD_PROTOGEN_DIR}/naples_status.pb.cc ${BLD_PROTOGEN_DIR}/naples_status.delphi.cc
include ${MKDEFS}/post.mk
