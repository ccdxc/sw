# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = sysmgr_delphidp.submake
MODULE_PIPELINE = iris gft apulu
MODULE_DEPS     := ${NICDIR}/sysmgr/proto/sysmgr.proto
include ${MKDEFS}/post.mk
