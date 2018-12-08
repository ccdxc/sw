# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKINFRA}/common.mk
include ${MKINFRA}/release.mk

CMD_GCC_NO_COV := gcc
CMD_GXX_NO_COV := g++

ifeq ($(COVERAGE),1)
    CMD_GCC := /home/asic/tools/eda/bullseye/bin/gcc
    CMD_GXX := /home/asic/tools/eda/bullseye/bin/g++
else
    CMD_GCC     := gcc
    CMD_GXX     := g++
endif

CMD_GXX_FLAGS       := ${COMMON_GXX_FLAGS} ${RELEASE_GXX_FLAGS}
CMD_GPP_FLAGS       := ${COMMON_GPP_FLAGS} ${RELEASE_GXX_FLAGS}

CMD_AS              := as
CMD_AR              := ar
CMD_AR_FLAGS        := ${COMMON_AR_FLAGS} ${RELEASE_AR_FLAGS}

CONFIG_ARLIB_FLAGS  := ${COMMON_ARLIB_FLAGS} ${RELEASE_ARLIB_FLAGS}
CONFIG_SOLIB_FLAGS  := ${COMMON_SOLIB_FLAGS} ${RELEASE_SOLIB_FLAGS}

CONFIG_INCS         := ${COMMON_INCS}
CONFIG_LDPATHS      := ${COMMON_LDPATHS}

# GTEST specific defines
CONFIG_GTEST_INCS               := ${COMMON_GTEST_INCS}
CONFIG_GTEST_LIBS               := ${COMMON_GTEST_LIBS} ${COMMON_GTEST_DIR}/make/gtest.a
CONFIG_GTEST_WOM_LIBS           := ${COMMON_GTEST_LIBS} ${COMMON_GTEST_DIR}/make/gtest_main.a
CONFIG_GTEST_LDPATHS            :=
CONFIG_GTEST_FLAGS              := ${COMMON_GTEST_FLAGS}
# For gtest, we can't use the following GCC flags.
CONFIG_GTEST_EXCLUDE_FLAGS      := ${COMMON_GTEST_EXCLUDE_FLAGS}

CMD_GOBIN           := go build 
