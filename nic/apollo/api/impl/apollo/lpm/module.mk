# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblpmitree_apollo.lib
MODULE_SRCS     = ${MODULE_SRC_DIR}/../../lpm/lpm.cc \
                  ${MODULE_SRC_DIR}/../../lpm/lpm_priv.cc \
                  $(wildcard ${MODULE_SRC_DIR}/*.cc)
MODULE_PIPELINE = apollo
include ${MKDEFS}/post.mk
