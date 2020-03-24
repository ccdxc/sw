# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsvendor_impl_ibm.lib
MODULE_PIPELINE = apulu
MODULE_SRCS     = $(wildcard ${MODULE_DIR}/*.c)                             \
                  $(wildcard ${MODULE_DIR}/*.cc)
MODULE_PREREQS  = vpp_pkg.export
MODULE_SOLIBS   = pdsvpp_impl
MODULE_LDLIBS   =
MODULE_INCS     = ${VPP_PLUGINS_INCS} ${MODULE_DIR}/${PIPELINE}
MODULE_DEFS     = ${VPP_DEFINES_${ARCH}}
MODULE_FLAGS    = ${VPP_FLAGS_${ARCH}}
include ${MKDEFS}/post.mk
