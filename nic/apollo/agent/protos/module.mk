# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET        = libpdsproto.lib
MODULE_PIPELINE      = apollo artemis apulu athena
MODULE_INCS          = /usr/local/include \
                       ${TOPDIR}/nic/hal/third-party/google/include \
                       ${BLD_PROTOGEN_DIR}
MODULE_LDLIBS        = pthread
MODULE_FLAGS         = -O3
MODULE_EXCLUDE_FLAGS = -O2
MODULE_SRCS          = $(wildcard ${BLD_PROTOGEN_DIR}/*.cc) \
                       $(wildcard ${BLD_PROTOGEN_DIR}/meta/*.cc)
include ${MKDEFS}/post.mk
