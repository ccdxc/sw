# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsdklinkmgr.lib
MODULE_SOLIBS   = sdkxcvrdriver
MODULE_DEFS     = -DNRMAKE
MODULE_SRCS     = ${MODULE_SRC_DIR}/linkmgr.cc  \
	          ${MODULE_SRC_DIR}/linkmgr_rw.cc \
	          ${MODULE_SRC_DIR}/linkmgr_state.cc \
	          ${MODULE_SRC_DIR}/port.cc
ifeq ($(ASIC),elba)
MODULE_SRCS     +=  ${MODULE_SRC_DIR}/port_serdes_elba.cc
MODULE_SRCS     +=  ${MODULE_SRC_DIR}/port_mac_elba.cc
MODULE_SRCS     +=  ${MODULE_SRC_DIR}/port_an_elba.cc
MODULE_FLAGS    = -DELBA
else
MODULE_SRCS     +=  ${MODULE_SRC_DIR}/port_serdes.cc
MODULE_SRCS     +=  ${MODULE_SRC_DIR}/port_mac.cc
MODULE_SRCS     +=  ${MODULE_SRC_DIR}/port_an_capri.cc
endif

include ${MKDEFS}/post.mk
