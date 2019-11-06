# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libftlitep4pd_mock.so
MODULE_FLAGS    = -fno-permissive
MODULE_FLAGS    = -O0
MODULE_PIPELINE = artemis apulu
include ${MKDEFS}/post.mk
