# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhal_src.so
#MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/hal.cc \
                  ${MODULE_SRC_DIR}/hal_state.cc \
                  ${MODULE_SRC_DIR}/hal_obj.cc \
                  ${MODULE_SRC_DIR}/hal_trace.cc \
                  $(wildcard ${MODULE_SRC_DIR}/src/stats/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/src/debug/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/src/internal/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/src/utils/*.cc)
MODULE_INCS     = ${TOPDIR}/nic/hal/third-party/openssl/include \
                  ${TOPDIR}/nic/hal/iris/deplhi
include ${MKDEFS}/post.mk
