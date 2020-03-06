# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libsdkp4.lib
MODULE_SRCS     = ${MODULE_SRC_DIR}/p4_utils.cc \
                  ${MODULE_SRC_DIR}/p4_api.cc \
                  ${MODULE_SRC_DIR}/p4pluspd_rxdma_api.cc \
                  ${MODULE_SRC_DIR}/p4pluspd_txdma_api.cc
include ${MKDEFS}/post.mk
