# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libevents_recorder.so
MODULE_SOLIBS = events_queue
MODULE_ARCH   = x86_64
include ${MKDEFS}/post.mk