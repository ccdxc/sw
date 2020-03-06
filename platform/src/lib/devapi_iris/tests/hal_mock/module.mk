# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_PIPELINE = iris gft
MODULE_TARGET = devapi_iris_mock.lib
MODULE_ARCH   = x86_64
MODULE_INCS     := ${BLD_PROTOGEN_DIR}/ ${MODULE_SRC_DIR}/../.. \
	               ${TOPDIR}/nic/sdk/platform/devapi
#MODULE_SOLIBS   := halproto delphisdk evutils shmmgr mmgr
MODULE_LDLIBS   := ${NIC_THIRDPARTY_GOOGLE_LDLIBS} rt
include ${MKDEFS}/post.mk
