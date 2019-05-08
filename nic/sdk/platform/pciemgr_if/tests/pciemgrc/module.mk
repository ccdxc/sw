# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := pciemgrc.gtest
MODULE_SOLIBS   := pciemgr_if evutils pal misc
MODULE_LDLIBS   := m ev
include ${MKDEFS}/post.mk
