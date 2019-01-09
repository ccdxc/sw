# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = diag_test
MODULE_SOLIBS   = diag sdkpal logger sensor sdkcapri_csrint
MODULE_LDLIBS   = dl ${NIC_COMMON_LDLIBS} ${NIC_CAPSIM_LDLIBS}
MODULE_INCS     := ${MODULE_SRC_DIR}/../ \
				   ${MODULE_SRC_DIR}/../../../lib/pal/ \
				   ${MODULE_SRC_DIR}/../../../lib/logger/
MODULE_FLAGS    = -DCAPRI_SW

include ${MKDEFS}/post.mk
