# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsaproto.so
MODULE_PIPELINE = apulu
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${TOPDIR}/hal/third-party/grpc/include
MODULE_FLAGS    = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_PREREQS  = pdsa.proto
MODULE_SRCS     = ${BLD_PROTOGEN_DIR}/pdsa.pb.cc
include ${MKDEFS}/post.mk
