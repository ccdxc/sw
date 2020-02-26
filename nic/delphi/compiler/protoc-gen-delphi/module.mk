# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = protoc-gen-delphi.gobin
MODULE_FLAGS    = -ldflags="-s -w"
MODULE_PIPELINE = iris gft apulu
MODULE_DEPS     = ${TOPDIR}/nic/delphi/compiler/delphic
MODULE_PREREQS  = protoc-gen-delphigo-metrics.gobin protoc-gen-delphigo.gobin

include ${MKDEFS}/post.mk
