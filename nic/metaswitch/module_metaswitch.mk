# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = metaswitch.submake
MODULE_PIPELINE = apulu
MODULE_ARCH     = x86_64
include ${MKDEFS}/post.mk
