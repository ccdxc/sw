# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKINFRA}/common.mk
include ${MKINFRA}/release.mk

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
					   -march=armv8-a+crc+crypto

ARCH_LINKER_FLAGS   := -Wl,--dynamic-linker=/lib/ld-linux-aarch64.so.1 \
                       --sysroot=${TOOLCHAIN_DIR}/aarch64-linux-gnu/libc  \
                       -B${TOOLCHAIN_DIR}/aarch64-linux-gnu/bin

CMD_GCC_NO_COV  := ${TOOLCHAIN_PREFIX}-gcc
CMD_GXX_NO_COV  := ${TOOLCHAIN_PREFIX}-g++
ifeq ($(COVERAGE),1)
    CMD_GCC    := /home/asic/tools/eda/bullseye/bin/aarch64-linux-gnu-gcc
    CMD_GXX    := /home/asic/tools/eda/bullseye/bin/aarch64-linux-gnu-g++
else
    CMD_GCC    := ${TOOLCHAIN_PREFIX}-gcc
    CMD_GXX    := ${TOOLCHAIN_PREFIX}-g++
endif

CMD_GXX_FLAGS       := ${COMMON_GXX_FLAGS} ${RELEASE_GXX_FLAGS} ${ARCH_GXX_FLAGS}
CMD_GPP_FLAGS       := ${COMMON_GPP_FLAGS} ${RELEASE_GXX_FLAGS} ${ARCH_GXX_FLAGS}

CMD_AR              := ${TOOLCHAIN_PREFIX}-ar
CMD_AR_FLAGS        := ${COMMON_AR_FLAGS} ${RELEASE_AR_FLAGS}

CMD_LD              := ${TOOLCHAIN_PREFIX}-ld
CMD_NM              := ${TOOLCHAIN_PREFIX}-nm
CMD_OBJCOPY         := ${TOOLCHAIN_PREFIX}-objcopy
CMD_OBJDUMP         := ${TOOLCHAIN_PREFIX}-objdump
CMD_STRIP           := ${TOOLCHAIN_PREFIX}-strip
CMD_AS              := ${TOOLCHAIN_PREFIX}-as

CONFIG_ARLIB_FLAGS  := ${COMMON_ARLIB_FLAGS}  ${RELEASE_ARLIB_FLAGS}
CONFIG_SOLIB_FLAGS  := ${COMMON_SOLIB_FLAGS} ${RELEASE_SOLIB_FLAGS}

ARCH_LDPATHS__      := ${TOOLCHAIN_DIR}/aarch64-linux-gnu/libc \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/lib \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/usr/lib \
                       ${TOOLCHAIN_DIR}/aarch64-linux-gnu/usr/lib/aarch64-linux-gnu
ARCH_LDPATHS        := $(addprefix -L,${ARCH_LDPATHS__})
CONFIG_LDPATHS      := ${COMMON_LDPATHS} ${ARCH_LDPATHS}

CONFIG_INCS         := ${COMMON_INCS}

# GTEST specific defines
CONFIG_GTEST_INCS               := ${COMMON_GTEST_INCS}
CONFIG_GTEST_LIBS               := ${COMMON_GTEST_LIBS} -lgtest -lgtest_without_main
CONFIG_GTEST_LDPATHS            := -L${COMMON_GTEST_DIR}/make/aarch64/
CONFIG_GTEST_FLAGS              := ${COMMON_GTEST_FLAGS}
# For gtest, we cant use the following GCC flags.
CONFIG_GTEST_EXCLUDE_FLAGS      := ${COMMON_GTEST_EXCLUDE_FLAGS}

CMD_GOBIN := CGO_LDFLAGS="-L/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/usr/include -L/usr/src/github.com/pensando/sw/nic/build/${ARCH}/${PIPELINE}/${ASIC}/lib" \
	CC=/tool/toolchain/aarch64-1.1/bin/aarch64-linux-gnu-gcc CGO_ENABLED=1 GOOS=linux GOARCH=arm64 go build
