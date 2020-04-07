# {C} Copyright 2020 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET        = fsm_test.bin
MODULE_PREREQS       = graceful_test.upgfsmgen hitless_test.upgfsmgen
MODULE_LDLIBS        = stdc++ m
MODULE_SOLIBS        = operd penipc thread event_thread utils upgrade_ev
MODULE_ARCH          = x86_64
include ${MKDEFS}/post.mk
