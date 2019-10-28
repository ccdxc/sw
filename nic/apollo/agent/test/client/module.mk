# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   	= libagentclient.so
MODULE_PIPELINE 	= apollo artemis apulu
MODULE_INCS          	= ${BLD_PROTOGEN_DIR}
MODULE_LDLIBS        	= ${NIC_THIRDPARTY_GOOGLE_LDLIBS}
MODULE_SRCS          	= $(wildcard ${MODULE_SRC_DIR}/*.cc) \
			  $(wildcard ${BLD_PROTOGEN_DIR}/*.cc) \
			  $(wildcard ${BLD_PROTOGEN_DIR}/meta/*.cc)
include ${MKDEFS}/post.mk
