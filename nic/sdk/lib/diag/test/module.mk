# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = diag_test
MODULE_SOLIBS   = diag sdkpal logger sensor
MODULE_LDLIBS   = dl
MODULE_INCS     := ${MODULE_SRC_DIR}/../ \
				   ${MODULE_SRC_DIR}/../../pal/ \
				   ${MODULE_SRC_DIR}/../../logger/
include ${MKDEFS}/post.mk
