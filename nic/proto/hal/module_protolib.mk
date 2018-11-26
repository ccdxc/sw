# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhalproto.so
#MODULE_PIPELINE = iris gft
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${TOPDIR}/hal/third-party/grpc/include
MODULE_LDLIBS   = pthread
MODULE_SOLIBS   = delphisdk sdkpal logger
MODULE_ARLIBS   = delphishm
MODULE_FLAGS    = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_SRC_DIR  = ${BLD_PROTOGEN_DIR}
include ${MKDEFS}/post.mk
