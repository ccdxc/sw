# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_PREREQS  := capviewdb.submake
MODULE_TARGET   := capview.bin
MODULE_SOLIBS   := pal
MODULE_INCS     := ${MODULE_SRC_DIR}/../include \
                    ${TOPDIR}/platform/src/third-party/lib/edit/include
MODULE_LDPATHS  := ${TOPDIR}/platform/src/third-party/lib/gmp/${ARCH} \
                    ${TOPDIR}/platform/src/third-party/lib/edit/${ARCH} \
                    ${TOPDIR}/platform/src/third-party/lib/ncurses/${ARCH}
MODULE_LDLIBS   := gmp edit ncurses
include ${MKDEFS}/post.mk
