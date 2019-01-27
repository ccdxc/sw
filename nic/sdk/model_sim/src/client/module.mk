# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libmodel_client.so
MODULE_INCS   = ${TOPDIR}/nic/hal/third-party/zmq/include
MODULE_LDLIBS = zmq
include ${MKDEFS}/post.mk
