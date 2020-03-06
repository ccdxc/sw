# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libevents_recorder.lib
MODULE_PIPELINE = iris gft
MODULE_SOLIBS   = events_queue eventtypes
include ${MKDEFS}/post.mk
