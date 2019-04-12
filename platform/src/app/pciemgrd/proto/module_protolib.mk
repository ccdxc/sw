# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpciemgrproto.so
MODULE_PIPELINE = iris gft
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${BLD_PROTOGEN_DIR}/proto \
                  ${TOPDIR}/hal/third-party/grpc/include
MODULE_FLAGS    = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS  = pciemgr.proto penctl.proto libpenctlproto.so
MODULE_SRCS     = ${BLD_PROTOGEN_DIR}/pciemgr/pciemgr.pb.cc \
                  ${BLD_PROTOGEN_DIR}/pciemgr/pciemgr.delphi.cc
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
MODULE_SOLIBS   = delphisdk penctlproto
include ${MKDEFS}/post.mk
