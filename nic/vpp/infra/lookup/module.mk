# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET    = infra_plugin.lib
MODULE_PIPELINE  = apollo artemis apulu
MODULE_PREREQS   = vpp_pkg.export
MODULE_SOLIBS    = pdsvpp_impl pdsvpp_api pdsvpp_ipc
MODULE_LDLIBS    = ${SDK_THIRD_PARTY_VPP_LIBS}
MODULE_INCS      = ${VPP_PLUGINS_INCS}
MODULE_DEFS      = ${VPP_DEFINES_${ARCH}}
MODULE_FLAGS     = ${VPP_FLAGS_${ARCH}}
include ${MKDEFS}/post.mk
