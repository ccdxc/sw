# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libociapi.so
MODULE_PIPELINE = apollo
MODULE_SRCS     = ${MODULE_SRC_DIR}/switchport.cc \
                  ${MODULE_SRC_DIR}/tep.cc \
                  ${MODULE_SRC_DIR}/vcn.cc \
                  ${MODULE_SRC_DIR}/subnet.cc \
                  ${MODULE_SRC_DIR}/vnic.cc \
                  ${MODULE_SRC_DIR}/mapping.cc \
                  ${MODULE_SRC_DIR}/batch.cc \
                  ${MODULE_SRC_DIR}/init.cc
MODULE_SOLIBS   = ociframework
include ${MKDEFS}/post.mk
