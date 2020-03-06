# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsysmon.lib
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc)
MODULE_FLAGS    = -DCAPRI_SW ${NIC_CSR_FLAGS} -O2
include ${MKDEFS}/post.mk
