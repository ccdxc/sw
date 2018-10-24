# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = linkmgr.bin
MODULE_PIPELINE = iris gft
MODULE_SRCS   = ${MODULE_SRC_DIR}/main.cc
MODULE_SOLIBS = linkmgr_src sdklinkmgr linkmgr_libsrc hal_lib \
                linkmgrcsr linkmgrdelphi capri_csrint \
                trace logger thread catalog sdkpal halproto mtrack ht \
                sdklinkmgr timerfd twheel periodic utils slab shmmgr list
MODULE_LDLIBS = pthread z dl m rt Judy ev :libprotobuf.so.14 sknobs \
                grpc++ grpc++_reflection \
                grpc++_unsecure grpc_unsecure AAPL
MODULE_FLAGS  = -pthread -rdynamic
include ${MKDEFS}/post.mk
