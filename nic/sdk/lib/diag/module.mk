# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libdiag.so
MODULE_INCS     := ${MODULE_SRC_DIR}/../../ \
				   ${MODULE_SRC_DIR}/../pal/ \
				   ${MODULE_SRC_DIR}/../logger/ \
				   ${MODULE_SRC_DIR}/../../../../platform/src/lib/sensor/
include ${MKDEFS}/post.mk
