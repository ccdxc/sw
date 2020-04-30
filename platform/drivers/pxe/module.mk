# {C} Copyright 2019 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   := ipxe.submake
MODULE_ARCH     := aarch64
MODULE_PIPELINE := iris apulu
MODULE_INCS     := ${TOPDIR}/nic/sdk/platform/drivers/
MODULE_DEPS      = $(wildcard ${TOPDIR}/platform/drivers/pxe/*.h) \
                   $(wildcard ${TOPDIR}/platform/drivers/pxe/*.c) \
                   $(wildcard ${TOPDIR}/platform/drivers/common/*.h) \
                   ${TOPDIR}/nic/sdk/platform/drivers/ionic_base.h
include ${MKDEFS}/post.mk
