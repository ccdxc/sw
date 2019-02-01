# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := capmon.bin
MODULE_SOLIBS   := pal
MODULE_INCS     := ${TOPDIR}/platform/src/third-party/lib/ncurses/include/
MODULE_LDPATHS	:= ${TOPDIR}/platform/src/third-party/lib/ncurses/${ARCH}
MODULE_LDLIBS   = ncurses

include ${MKDEFS}/post.mk
