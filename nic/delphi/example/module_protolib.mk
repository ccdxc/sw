# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libdelphiexampleproto.lib
MODULE_PIPELINE = iris
MODULE_ARCH     = x86_64
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${TOPDIR}/hal/third-party/grpc/include
MODULE_FLAGS    = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS  = delphi_example.proto
MODULE_ARLIBS   = delphishm
MODULE_SRCS     = ${BLD_PROTOGEN_DIR}/example.pb.cc ${BLD_PROTOGEN_DIR}/example.delphi.cc \
                  ${BLD_PROTOGEN_DIR}/example_stats.pb.cc ${BLD_PROTOGEN_DIR}/example_stats.delphi.cc
include ${MKDEFS}/post.mk
