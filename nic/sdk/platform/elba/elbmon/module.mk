# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ASIC     := elba
MODULE_TARGET   := elbmon.bin
MODULE_SOLIBS   := pal
MODULE_FLAGS = -g

include ${MKDEFS}/post.mk
