COMMON_DEPS             := $(wildcard makedefs/*) ${TOP_MAKEFILE}
COMMON_INCS__           := ${TOPDIR}
COMMON_INCS             := $(addprefix -I,${COMMON_INCS__})

COMMON_LDPATHS__        := ${BLD_LIB_DIR}
COMMON_LDPATHS          := $(addprefix -L,${COMMON_LDPATHS__})

# GTEST specific defines
COMMON_GTEST_INCS       := -Ithird-party/googletest-release-1.8.0/googletest/include/
COMMON_GTEST_LIBS       := third-party/googletest-release-1.8.0/googletest/make/gtest.a -lpthread
COMMON_GTEST_LDPATHS    := -Lthird-party/googletest-release-1.8.0/googletest/make/
