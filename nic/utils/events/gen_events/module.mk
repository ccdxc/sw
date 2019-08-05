# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = gen_events.bin
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/main.cc
MODULE_SOLIBS   = eventproto eventtypes ipc events_queue events_recorder
MODULE_LDLIBS   = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} ${NIC_COMMON_LDLIBS}
include ${MKDEFS}/post.mk