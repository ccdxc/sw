# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libfte.lib
MODULE_PIPELINE = iris gft
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/acl/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/utils/*.cc) 
MODULE_INCS     = ${BLD_PROTOGEN_DIR}/
MODULE_SOLIBS   = sdkpal delphisdk halproto shmmgr
include ${MKDEFS}/post.mk
