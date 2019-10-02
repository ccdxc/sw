# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = libpenipc.so
MODULE_SOLIBS = 
MODULE_LDLIBS = zmq
MODULE_FLAGS  = -Werror
include ${MKDEFS}/post.mk
