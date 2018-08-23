# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = libsdklinkmgr.so
MODULE_DEFS     = -DNRMAKE
MODULE_INCS     = ../third-party/boost/include/
include ${MAKEDEFS}/post.mk
