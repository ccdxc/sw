# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libupgradesvc.so
MODULE_PIPELINE = apulu
MODULE_INCS     = ${MODULE_GEN_DIR}
MODULE_PREREQS  = pdsupgradegen.proto
include ${MKDEFS}/post.mk
