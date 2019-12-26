# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = arp_proxy.so
MODULE_PIPELINE = apulu
MODULE_ARCH     = aarch64 x86_64
MODULE_PREREQS  = vpp_pkg.export
MODULE_SOLIBS   = pdsvpp_impl pdsvpp_api
MODULE_INCS     = ${VPP_PLUGINS_INCS}
MODULE_LDLIBS   = ${SDK_THIRD_PARTY_VPP_LIBS} pthread
MODULE_LDPATHS  = ${SDK_THIRD_PARTY_VPP_LDPATH}
MODULE_DEFS     = ${VPP_DEFINES_${ARCH}}
MODULE_FLAGS    = ${VPP_FLAGS_${ARCH}}
include ${MKDEFS}/post.mk
