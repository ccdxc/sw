# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET = hal_access.bin
MODULE_SOLIBS = pal logger
MODULE_LDLIBS = dl
include ${MAKEDEFS}/post.mk
