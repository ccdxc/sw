# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/common.mk
include ${MAKEDEFS}/release.mk

CMD_GXX             := g++
CMD_GXX_FLAGS       := ${COMMON_GXX_FLAGS} ${RELEASE_GXX_FLAGS}

CMD_AR              := ar
CMD_AR_FLAGS        := ${COMMON_AR_FLAGS} ${RELEASE_AR_FLAGS}

CONFIG_ARLIB_FLAGS  := ${COMMON_ARLIB_FLAGS} ${RELEASE_ARLIB_FLAGS}
CONFIG_SOLIB_FLAGS  := ${COMMON_SOLIB_FLAGS} ${RELEASE_SOLIB_FLAGS}

CONFIG_INCS         := ${COMMON_INCS}
CONFIG_LDPATHS      := ${COMMON_LDPATHS}

# GTEST specific defines
CONFIG_GTEST_INCS               := ${COMMON_GTEST_INCS}
CONFIG_GTEST_LIBS               := ${COMMON_GTEST_LIBS} ${COMMON_GTEST_DIR}/make/gtest.a
CONFIG_GTEST_LDPATHS            :=
CONFIG_GTEST_FLAGS              := ${COMMON_GTEST_FLAGS}
# For gtest, we cant use the following GCC flags.
CONFIG_GTEST_GCC_EXCLUDE_FLAGS  := ${COMMON_GTEST_GCC_EXCLUDE_FLAGS}

