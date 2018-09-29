# {C} Copyright 2018 Pensando Systems Inc. All rights reserved.
include ${MKINFRA}/post.mk
APPEND_COMMON := false

ifeq "$(suffix ${MODULE_TARGET})" ".bin"
    APPEND_COMMON = true
else ifeq "$(suffix ${MODULE_TARGET})" ".gtest"
    ifeq "${MODULE_ARCH}" "x86_64 aarch64"
        MODULE_ARCH := x86_64
    endif
    APPEND_COMMON = true
endif

ifeq "${APPEND_COMMON}" "true"
    MODULE_FLAGS    += ${NIC_COMMON_FLAGS}
endif

ifeq "$(suffix ${MODULE_TARGET})" ".p4bin"
    MODULE_DEPS += ${NIC_P4_NCC_DEPS}
endif
