# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = libevent_thread.lib
MODULE_SOLIBS = lfq thread penipc
MODULE_LDLIBS = ev
MODULE_FLAGS  = -Werror
include ${MKDEFS}/post.mk
