# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
ifeq "${MODULE_TARGET}" ""
    $(error MODULE_TARGET is not defined in ${MODULE_MK})
endif

ifeq "${MODULE_SRCS}" ""
    ifeq "$(suffix ${MODULE_TARGET})" ".p4bin"
        MODULE_SRCS = $(strip $(notdir $(wildcard ${MODULE_DIR}/*.p4)))
    else
        MODULE_SRCS = $(strip $(notdir $(wildcard ${MODULE_DIR}/*.cc)))
    endif
endif
