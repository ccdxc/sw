# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
include $(TOPDIR)/nic/metaswitch/pre.mk
MODULE_TARGET   = metaswitch.submake
MODULE_PIPELINE = apollo artemis apulu
MODULE_CLEAN_DIRS  = ${TOPDIR}/nic/third-party/metaswitch/buildcfg \
                     ${TOPDIR}/nic/third-party/metaswitch/output
include ${MKDEFS}/post.mk
