# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libfte.so
MODULE_PIPELINE = iris gft
MODULE_SRCS     = $(wildcard ${MODULE_SRC_DIR}/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/acl/*.cc) \
                  $(wildcard ${MODULE_SRC_DIR}/utils/*.cc)
include ${MKDEFS}/post.mk
