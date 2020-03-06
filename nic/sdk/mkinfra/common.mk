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
                           -Wunused-but-set-parameter \
                           -Wno-free-nonheap-object \
                           -Wno-builtin-macro-redefined \
                           -MD
ifeq ($(COVERAGE),0)
    COMMON_GXX_FLAGS        += -Wuninitialized
    COMMON_GXX_FLAGS        += -Werror
endif

ifeq ($(PLATFORM),haps)
    COMMON_GXX_FLAGS        += -DHAPS
else ifeq ($(PLATFORM),hw)
    COMMON_GXX_FLAGS        += -DHW
else ifeq ($(PLATFORM),sim)
    COMMON_GXX_FLAGS        += -DSIM
endif

ifeq ($(PERF),1)
    COMMON_GXX_FLAGS        += -DPERF
endif

# pipeline specific stuff inside SDK is big no-no, new pipeline is broken now
# @lseshan please follow up and fix this
COMMON_GXX_FLAGS		+= -DPIPELINE=${PIPELINE}
ifeq ($(PIPELINE), iris)
	COMMON_GXX_FLAGS        += -DENABLE_SDK_MTRACK
endif

COMMON_GPP_FLAGS        := ${COMMON_GXX_FLAGS} --std=c++11
COMMON_AR_FLAGS         := rcs
COMMON_ARLIB_FLAGS      :=
COMMON_SOLIB_FLAGS      := -shared

COMMON_DEPS             := $(wildcard ${MKINFRA}/*) ${TOP_MAKEFILE}
COMMON_INCS__           := ${TOPDIR} ${SDKDIR} ${BLD_ARCH_DIR} ${THIRD_PARTY_INCLUDES} ${NIC_CSR_INCS}
COMMON_INCS             := $(addprefix -I,${COMMON_INCS__})

RPATH_PREFIX            := -Wl,-rpath,
COMMON_LDPATHS__        := ${BLD_LIB_DIR}
COMMON_LDPATHS          := $(addprefix -L,${COMMON_LDPATHS__}) \
                           $(addprefix ${RPATH_PREFIX},${COMMON_LDPATHS__})

# GTEST specific defines
COMMON_GTEST_DIR                = ${SDKDIR}/third-party/googletest-release-1.8.0/googletest/
COMMON_GTEST_INCS               := -I${COMMON_GTEST_DIR}/include/
COMMON_GTEST_LIBS               := -lpthread
COMMON_GTEST_LDPATHS            := -L${COMMON_GTEST_DIR}/make/
COMMON_GTEST_FLAGS              := -Wno-sign-compare
# For gtest, we cant use the following GCC flags.
COMMON_GTEST_EXCLUDE_FLAGS      := -Werror

NAME_GCC       := "[  GCC   ]"
NAME_GXX       := "[  G++   ]"
NAME_SYMLINK   := "[SYMLINK ]"
NAME_NCC       := "[  NCC   ]"
NAME_CAPAS     := "[ CAPAS  ]"
NAME_PROTOC    := "[ PROTOC ]"
NAME_SVCGEN    := "[ SVCGEN ]"
NAME_PROT2CC   := "[PROTO2CC]"
NAME_PROT2PY   := "[PROTO2PY]"
NAME_PROT2GO   := "[PROTO2GO]"
NAME_PROT2C    := "[ PROTO2C]"
NAME_PROT2DELPHI := "[PROTO2DELPHI]"
NAME_PROT2GOMETRICS := "[PROTO2GOMETRICS]"
NAME_PROT2PENCTL := "[PROTO2PENCTL]"
NAME_PROT2PDSASVC := "[PROTO2PDSASVC]"
NAME_PROT2PDSAMGMT := "[PROTO2PDSAMGMT]"
NAME_MOCKGEN   := "[MOCKGEN ]"
NAME_MKTARGET  := "[MKTARGET]"
NAME_GOIMPORT  := "[GOIMPORT]"
NAME_TENJIN    := "[ TENJIN ]"
NAME_GOBUILD   := "[GO BUILD]"
NAME_SUBMAKE   := "[SUB MAKE]"
NAME_SWIGCLI   := "[SWIGCLI ]"
NAME_UPGFSMGEN    := "[UPGFSMGEN]"
NAME_MEMRGNS    := "[MEMRGNS ]"
NAME_UPX	:= "[UPX COMPRESSION ]"
NAME_SORRENTO   := "[SORRENTO ]"

COMMON_CMD_NCC              := ${TOPDIR}/nic/tools/ncc/capri-ncc.py
COMMON_CMD_SORRENTO         := ${TOPDIR}/nic/tools/sorrento/bin/p4c-sorrento
COMMON_P4C_OPTS             :=

COMMON_CMD_CAPAS            := ${SDKDIR}/third-party/asic/capri/model/capsim-gen/bin/capas
COMMON_CAPAS_OPTS           := -t
ifeq ($(PLATFORM),haps)
    COMMON_CAPAS_OPTS       += -DHAPS
endif

ifeq ($(PLATFORM),hw)
    COMMON_CAPAS_OPTS       += -DHW
endif

ifeq ($(COVERAGE), 1)
    COMMON_CAPAS_OPTS       += -C
endif
