# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
ifeq "${MODULE_TARGET}" ""
    $(error MODULE_TARGET is not defined in ${MODULE_MK})
endif

ifeq "${MODULE_SRCS}" ""
    ifeq "$(suffix ${MODULE_TARGET})" ".p4bin"
        MODULE_SRCS = $(wildcard ${MODULE_SRC_DIR}/*.p4)
    else ifeq "$(suffix ${MODULE_TARGET})" ".asmbin"
        MODULE_SRCS = $(wildcard ${MODULE_SRC_DIR}/*.asm)
        MODULE_SRCS += $(wildcard ${MODULE_SRC_DIR}/*.s)
    else ifeq "$(suffix ${MODULE_TARGET})" ".proto"
        MODULE_SRCS = $(sort $(wildcard ${MODULE_SRC_DIR}/*.proto))
    else ifeq "$(suffix ${MODULE_TARGET})" ".mockgen"
        MODULE_SRCS = $(sort $(wildcard ${MODULE_SRC_DIR}/*.pb.go))
    else ifeq "$(suffix ${MODULE_TARGET})" ".goimports"
        MODULE_SRCS = $(sort $(wildcard ${MODULE_SRC_DIR}/*mock.go))
    else ifeq "$(suffix ${MODULE_TARGET})" ".swigcli"
        MODULE_SRCS = $(sort $(wildcard ${MODULE_SRC_DIR}/*.i))
    else
        MODULE_SRCS = $(wildcard ${MODULE_SRC_DIR}/*.cc) $(wildcard ${MODULE_SRC_DIR}/*.c)
    endif
endif

ifeq "${MODULE_GEN_DIR}" ""
    MODULE_GEN_DIR := ${BLD_PROTOGEN_DIR}
endif

ifeq "$(suffix ${MODULE_TARGET})" ".export"
    ifeq "${MODULE_EXPORT_LIBS}" ""
        MODULE_EXPORT_LIBS = $(wildcard ${MODULE_EXPORT_DIR}/lib/*.so*) \
                             $(wildcard ${MODULE_EXPORT_DIR}/lib/*.a)
    endif
    ifeq "${MODULE_EXPORT_BINS}" ""
        MODULE_EXPORT_BINS = $(wildcard ${MODULE_EXPORT_DIR}/bin/*)
    endif
endif
