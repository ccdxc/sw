# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = nmd_delphi.submake
MODULE_PIPELINE = iris
MODULE_DEPS     := ${NICDIR}/agent/nmd/protos/delphi/naples_status.proto
include ${MKDEFS}/post.mk
