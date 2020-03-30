# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libalerts_operd_con.lib
MODULE_PIPELINE = apulu
MODULE_SOLIBS   = pdsproto
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt pthread
MODULE_ARLIBS   =
MODULE_CFLAGS   = -Werror
MODULE_INCS     = /usr/local/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${TOPDIR}/hal/third-party/grpc/include \
		  ${MODULE_GEN_DIR}
ALL_CC_FILES    = $(wildcard ${MODULE_SRC_DIR}/*.cc)
ALL_TEST_FILES  = $(wildcard ${MODULE_SRC_DIR}/*_test.cc)
MODULE_SRCS     = $(filter-out $(ALL_TEST_FILES), $(ALL_CC_FILES))
include ${MKDEFS}/post.mk
