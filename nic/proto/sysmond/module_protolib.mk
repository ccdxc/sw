# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsysmondproto.lib
MODULE_PIPELINE = iris gft
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${BLD_PROTOGEN_DIR}/proto \
                  ${TOPDIR}/hal/third-party/grpc/include
MODULE_FLAGS    = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS  = sysmond.proto penctl.proto libpenctlproto.lib
MODULE_SRCS     = ${BLD_PROTOGEN_DIR}/sysmond.pb.cc \
                  ${BLD_PROTOGEN_DIR}/sysmond.delphi.cc
MODULE_SOLIBS   = penctlproto
include ${MKDEFS}/post.mk
