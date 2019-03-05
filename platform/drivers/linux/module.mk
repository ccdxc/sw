# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_ARCH     := aarch64
MODULE_TARGET   := mnic.submake
export AARCH64_LINUX_HEADERS	:= ${NICDIR}/buildroot/output/linux-headers
include ${MKDEFS}/post.mk
