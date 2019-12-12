# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libpdsnicmgr.so
MODULE_PIPELINE = apollo artemis apulu poseidon
MODULE_INCS     := ${MODULE_SRC_DIR} \
                   ${TOPDIR}/nic/sdk/platform/devapi
MODULE_DEPS     = hal.memrgns
include ${MKDEFS}/post.mk
