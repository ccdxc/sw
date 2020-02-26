# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = protoc-gen-delphigo-metrics.gobin
MODULE_PIPELINE = iris gft apulu
MODULE_FLAGS    = -ldflags="-s -w"
MODULE_DEPS = ${TOPDIR}/nic/delphi/compiler/delphic
include ${MKDEFS}/post.mk
