# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libplugin_ep_learn.lib
MODULE_PIPELINE = iris gft
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/arp/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/dhcp/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/common/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/data_pkt/*.cc)
MODULE_INCS     = ${TOPDIR}/nic/hal/third-party/isc/include
include ${MKDEFS}/post.mk
