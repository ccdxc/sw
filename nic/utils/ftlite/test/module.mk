# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

# TODO , Need to remove the pipeline dependent code. Un-used now
include ${MKDEFS}/pre.mk
MODULE_TARGET   = ftlite_test.bin
MODULE_SOLIBS   = ftlite ftlitep4pd_mock logger
MODULE_FLAGS    = -O3
MODULE_PIPELINE = apollo
include ${MKDEFS}/post.mk
