# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libociapi_impl.so
MODULE_PIPELINE = apollo
MODULE_SRCS     = ${MODULE_SRC_DIR}/mapping_impl.cc \
                  ${MODULE_SRC_DIR}/switchport_impl.cc
include ${MKDEFS}/post.mk
