# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_PREREQS  := capviewdb.submake
MODULE_TARGET   := capview.bin
MODULE_SOLIBS   := pal
MODULE_INCS     := ${MODULE_SRC_DIR}/../include
MODULE_LDLIBS   := gmp edit ncurses
include ${MKDEFS}/post.mk
