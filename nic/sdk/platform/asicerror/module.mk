# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libasicerror.lib
MODULE_SRCS     = ${MODULE_SRC_DIR}/$(ASIC)/asicerror.cc ${MODULE_SRC_DIR}/$(ASIC)/interrupts.cc
ifeq ($(ASIC),elba)
MODULE_FLAGS    = ${NIC_CSR_FLAGS} -O2 -DELBA -I${MODULE_SRC_DIR}
else
MODULE_FLAGS    = ${NIC_CSR_FLAGS} -O2 -I${MODULE_SRC_DIR}
endif
include ${MKDEFS}/post.mk
