# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := x86_64
MODULE_TARGET   := pciemgrs.gtest
MODULE_SOLIBS   := pciemgr_if pciemgrutils pciehdevices intrutils cfgspace misc pal evutils
MODULE_LDLIBS   := m ev
include ${MKDEFS}/post.mk
