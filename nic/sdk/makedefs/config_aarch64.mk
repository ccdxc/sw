# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/common.mk
include ${MAKEDEFS}/release.mk

TOOLCHAIN_DIR       = /tool/toolchain/aarch64-1.1
TOOLCHAIN_PREFIX    = ${TOOLCHAIN_DIR}/bin/aarch64-linux-gnu
ARCH_SYS_PATHS      := ${TOOLCHAIN_DIR}/aarch64-linux-gnu/include/c++/6.4.1/aarch64-linux-gnu \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/include/c++/6.4.1 \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/include \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/libc/usr/include \
                       ${TOOLCHAIN_DIR}/include \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/usr/include \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/usr/local/include \
                       ${TOOLCHAIN_DIR}/lib/gcc/aarch64-linux-gnu/6.4.1/include \
                       ${TOOLCHAIN_DIR}/lib/gcc/aarch64-linux-gnu/6.4.1/include-fixed
ARCH_GXX_FLAGS      := $(addprefix -isystem ,${ARCH_SYS_PATHS}) \
                       --sysroot=${TOOLCHAIN_DIR}/aarch64-linux-gnu/libc \
                       -no-canonical-prefixes \
                       -nostdinc \
                       -ffunction-sections \
                       -fdata-sections \
                       -fPIE

CMD_GXX             := ${TOOLCHAIN_PREFIX}-g++
CMD_GXX_FLAGS       := ${COMMON_GXX_FLAGS} ${RELEASE_GXX_FLAGS} ${ARCH_GXX_FLAGS}

CMD_AR              := ${TOOLCHAIN_PREFIX}-ar
CMD_AR_FLAGS        := ${COMMON_AR_FLAGS} ${RELEASE_AR_FLAGS}

CMD_LD				:= ${TOOLCHAIN_PREFIX}-ld
CMD_NM				:= ${TOOLCHAIN_PREFIX}-nm
CMD_OBJCOPY			:= ${TOOLCHAIN_PREFIX}-objcopy
CMD_OBJDUMP         := ${TOOLCHAIN_PREFIX}-objdump
CMD_STRIP           := ${TOOLCHAIN_PREFIX}-strip
CMD_AS              := ${TOOLCHAIN_PREFIX}-as

CONFIG_ARLIB_FLAGS  := ${COMMON_ARLIB_FLAGS}  ${RELEASE_ARLIB_FLAGS}

ARCH_LDPATHS        := ${TOOLCHAIN_DIR}/aarch64-linux-gnu/libc \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/lib \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/usr/lib \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/usr/lib/aarch64-linux-gnu \

ARCH_SOLIB_FLAGS    := $(addprefix -L,${ARCH_LDPATHS})
CONFIG_SOLIB_FLAGS  := ${COMMON_SOLIB_FLAGS} ${RELEASE_SOLIB_FLAGS} ${ARCH_SOLIB_FLAGS}

CONFIG_INCS         := ${COMMON_INCS}
CONFIG_LDPATHS      := ${COMMON_LDPATHS}

# GTEST specific defines
CONFIG_GTEST_INCS               := ${COMMON_GTEST_INCS}
CONFIG_GTEST_LIBS               := ${COMMON_GTEST_LIBS} -lgtest -lgtest_without_main
CONFIG_GTEST_LDPATHS            := -L${COMMON_GTEST_DIR}/make/aarch64/
CONFIG_GTEST_FLAGS              := ${COMMON_GTEST_FLAGS}
# For gtest, we cant use the following GCC flags.
CONFIG_GTEST_GCC_EXCLUDE_FLAGS  := ${COMMON_GTEST_GCC_EXCLUDE_FLAGS}

