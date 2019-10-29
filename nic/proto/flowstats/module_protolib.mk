# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libflowstatsproto.so
MODULE_PIPELINE = iris gft
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${TOPDIR}/hal/third-party/grpc/include
MODULE_FLAGS    = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS  = flowstats.proto
MODULE_SRCS     = ${BLD_PROTOGEN_DIR}/flowstats/flowstats.pb.cc \
                  ${BLD_PROTOGEN_DIR}/flowstats/flowstats.delphi.cc 
MODULE_INCS     = ${BLD_PROTOGEN_DIR}
MODULE_SOLIBS   = delphisdk
include ${MKDEFS}/post.mk
