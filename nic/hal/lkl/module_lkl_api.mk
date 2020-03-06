# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblkl_api.lib
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/lkl_api.cc \
                  ${MODULE_SRC_DIR}/lklshim.cc \
                  ${MODULE_SRC_DIR}/lklshim_v6.cc
MODULE_DEFS     = -DCONFIG_AUTO_LKL_POSIX_HOST
MODULE_INCS     = ${TOPDIR}/nic/hal/third-party/lkl/export/include
include ${MKDEFS}/post.mk
