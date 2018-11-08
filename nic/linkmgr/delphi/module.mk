# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblinkmgrdelphi.so
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/linkmgr_delphi.cc
MODULE_SOLIBS   = delphisdk
MODULE_PREREQS = libhalproto.so
include ${MKDEFS}/post.mk

