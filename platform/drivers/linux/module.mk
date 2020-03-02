# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := mnic.submake
MODULE_DEPS      = $(wildcard ${TOPDIR}/platform/drivers/linux/eth/ionic/*) \
                   $(wildcard ${TOPDIR}/platform/drivers/linux/mnet_uio_pdrv_genirq/*) \
                   $(wildcard ${TOPDIR}/platform/drivers/linux/mnet/*)
export AARCH64_LINUX_HEADERS	?= ${NICDIR}/buildroot/output/${ASIC}/linux-headers
include ${MKDEFS}/post.mk
