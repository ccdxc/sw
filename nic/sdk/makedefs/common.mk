# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
# 
# Note:
# This file is used to define all the flags and options common to
# all the cpu architectures.

COMMON_GXX_FLAGS        := -fstack-protector \
                           -fno-omit-frame-pointer \
                           -fno-canonical-system-headers \
                           -fdiagnostics-color=always \
                           -fPIC \
                           -U_FORTIFY_SOURCE \
                           -Wall \
                           -Werror\
                           -Wunused-but-set-parameter \
                           -Wno-free-nonheap-object \
                           -Wuninitialized \
                           -Wno-builtin-macro-redefined \
                           '-std=c++0x' '--std=c++11'\
                           -MD
COMMON_AR_FLAGS         := rcs
COMMON_ARLIB_FLAGS      := 
COMMON_SOLIB_FLAGS      := -shared

COMMON_DEPS             := #$(wildcard ${MAKEDEFS}/*) ${TOP_MAKEFILE}
COMMON_INCS__           := ${TOPDIR} ${SDKDIR}
COMMON_INCS             := $(addprefix -I,${COMMON_INCS__})

COMMON_LDPATHS__        := ${BLD_LIB_DIR}
COMMON_LDPATHS          := $(addprefix -L,${COMMON_LDPATHS__})

# GTEST specific defines
COMMON_GTEST_DIR                = ${SDKDIR}/third-party/googletest-release-1.8.0/googletest/
COMMON_GTEST_INCS               := -I${COMMON_GTEST_DIR}/include/
COMMON_GTEST_LIBS               := -lpthread
COMMON_GTEST_LDPATHS            := -L${COMMON_GTEST_DIR}/make/
COMMON_GTEST_FLAGS              := -Wno-sign-compare
# For gtest, we cant use the following GCC flags.
COMMON_GTEST_GCC_EXCLUDE_FLAGS  := -Werror

COMMON_GCC_SHORT_NAME       := "[  GCC  ]"
COMMON_ARLIB_SHORT_NAME     := "[ ARLIB ]"
COMMON_SOLIB_SHORT_NAME     := "[ SOLIB ]"
COMMON_BIN_SHORT_NAME       := "[  BIN  ]"
COMMON_SYMLINK_SHORT_NAME   := "[SYMLINK]"
COMMON_NCC_SHORT_NAME       := "[  NCC  ]"
COMMON_P4BIN_SHORT_NAME     := "[ P4BIN ]"
COMMON_CAPAS_SHORT_NAME     := "[ CAPAS ]"
COMMON_ASMBIN_SHORT_NAME    := "[ ASMBIN]"

COMMON_CMD_NCC              := ${TOPDIR}/nic/tools/ncc/capri-ncc.py
COMMON_NCC_OPTS             :=

COMMON_CMD_CAPAS            := ${TOPDIR}/nic/asic/capri/model/capsim-gen/bin/capas
COMMON_CAPAS_OPTS           := -t
ifneq "${GCOV}" ""
    COMMON_CAPAS_OPTS       += -C
endif
