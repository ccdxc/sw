# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET = gflags.export
MODULE_ARCH   = x86_64 aarch64
MODULE_PIPELINE = iris gft
include ${MKDEFS}/post.mk
