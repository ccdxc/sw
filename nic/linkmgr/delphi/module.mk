# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblinkmgrdelphi.lib
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/linkmgr_delphi.cc
MODULE_SOLIBS   = delphisdk events_recorder
MODULE_PREREQS = libhalproto.lib
include ${MKDEFS}/post.mk

