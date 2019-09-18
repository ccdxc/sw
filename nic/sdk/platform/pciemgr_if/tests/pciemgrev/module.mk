# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := pciemgrev.gtest
MODULE_SOLIBS   := pciemgr_if evutils pal misc
MODULE_LDLIBS   := m ev
include ${MKDEFS}/post.mk
