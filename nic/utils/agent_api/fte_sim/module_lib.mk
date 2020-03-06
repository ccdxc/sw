# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libfte_sim_lib.lib
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/fwlogsim_svc.cc
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
