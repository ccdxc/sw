# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libisc_dhcp.lib
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/common/alloc.c \
                  ${MODULE_SRC_DIR}/common/options.c \
                  ${MODULE_SRC_DIR}/common/parse.c \
                  ${MODULE_SRC_DIR}/common/tables.c \
                  ${MODULE_SRC_DIR}/common/tree.c \
                  ${MODULE_SRC_DIR}/dhcp_pen_api.c \
                  ${MODULE_SRC_DIR}/omapip/alloc.c \
                  ${MODULE_SRC_DIR}/omapip/errwarn.c \
                  ${MODULE_SRC_DIR}/omapip/hash.c
MODULE_FLAGS    = -x c++ --std=c++11 -Wno-misleading-indentation \
                  -Wno-unused-variable
MODULE_INCS     = ${TOPDIR}/nic/hal/third-party/isc/include
include ${MKDEFS}/post.mk
