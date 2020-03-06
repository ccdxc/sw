# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET        = libeventtypes.lib
MODULE_PIPELINE      = iris gft
MODULE_INCS          = /usr/local/include \
                       ${BLD_PROTOGEN_DIR} \
                       ${TOPDIR}/nic/hal/third-party/google/include \
                       ${TOPDIR}/hal/third-party/grpc/include
MODULE_FLAGS         = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS       = eventtypes.proto
MODULE_SRCS          = ${BLD_PROTOGEN_DIR}/attributes.pb.cc \
                       ${BLD_PROTOGEN_DIR}/eventtypes.pb.cc \
                       ${BLD_PROTOGEN_DIR}/gogo.pb.cc
include ${MKDEFS}/post.mk
