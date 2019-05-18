# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libftlp4pd_mock_ipv4.so
MODULE_ARCH     = x86_64
MODULE_FLAGS    = -fno-permissive
MODULE_FLAGS    = -O3
MODULE_PIPELINE = apollo iris
include ${MKDEFS}/post.mk
