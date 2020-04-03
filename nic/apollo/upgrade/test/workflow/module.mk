# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET        = pdsupgtesteventsvc.bin
MODULE_PREREQS       = graceful_test.upgfsmgen hitless_test.upgfsmgen
MODULE_PIPELINE      = apulu
MODULE_INCS          = ${MODULE_GEN_DIR}
MODULE_LDLIBS        = stdc++ m
MODULE_SOLIBS        = operd pdsupgproto penipc \
                       thread event_thread utils logger upgrade_ev
MODULE_LDLIBS        = ${NIC_THIRDPARTY_GOOGLE_LDLIBS} \
                       ${NIC_COMMON_LDLIBS} edit ncurses
MODULE_SRCS          = $(wildcard ${MODULE_SRC_DIR}/*.cc)
include ${MKDEFS}/post.mk
