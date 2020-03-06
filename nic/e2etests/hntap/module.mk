# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhntap_lib.lib
MODULE_ARCH     = x86_64
MODULE_SRCS     = ${MODULE_SRC_DIR}/dev.cc \
                  ${MODULE_SRC_DIR}/hntap.cc \
                  ${MODULE_SRC_DIR}/hntap_switch.cc
MODULE_INCS     = ${TOPDIR}/nic/hal/third-party/openssl/include
include ${MKDEFS}/post.mk
