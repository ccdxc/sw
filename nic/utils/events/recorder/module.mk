# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET = libevents_recorder.so
MODULE_PIPELINE = iris gft
MODULE_SOLIBS = events_queue
include ${MKDEFS}/post.mk