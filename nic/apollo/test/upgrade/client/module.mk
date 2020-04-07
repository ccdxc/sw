# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET        = pdsupgclient.bin
MODULE_PIPELINE      = apulu
MODULE_INCS          = /usr/local/include \
                       ${TOPDIR}/nic/hal/third-party/google/include \
                       ${BLD_PROTOGEN_DIR}
MODULE_LDLIBS        = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                       ${NIC_COMMON_LDLIBS}
MODULE_SRCS          = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                       $(wildcard ${BLD_PROTOGEN_DIR}/*.cc) \
                       $(wildcard ${BLD_PROTOGEN_DIR}/meta/*.cc)
include ${MKDEFS}/post.mk
