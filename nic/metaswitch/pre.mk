# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

MODE = release
MS_ROOT = $(TOPDIR)/nic/third-party/metaswitch
MS_LIB_DIR = $(MS_ROOT)/output/$(GMK_TARGETOS)
MS_COMPILATION_SWITCH = \
        NBB_64BIT_TYPES_NATIVE \
        NBB_64_BIT_POINTERS

ifeq ($(ARCH),aarch64)
    GMK_TARGETOS = aarch64
    MS_COMPILATION_SWITCH += LINUX_MT_NBASE
    MS_LIB_DIR += \
	-Wl,-rpath,$(TOPDIR)/nic/third-party/libxml2/$(GMK_TARGETOS)/lib \
        -Wl,-rpath,$(TOPDIR)/nic/third-party/libz/$(GMK_TARGETOS)/lib \
        -Wl,-rpath,$(TOPDIR)/nic/third-party/liblzma/$(GMK_TARGETOS)/lib \
        -Wl,-rpath,$(TOPDIR)/nic/third-party/libicuuc/$(GMK_TARGETOS)/lib
else
    GMK_TARGETOS = x86_64
    MS_COMPILATION_SWITCH += LINUX_MT_NBASE
endif

ifeq ($(MODE),debug)
    MS_COMPILATION_SWITCH += NBB_DEBUG
endif

MS_INCLPATH = \
	code/comn/intface        \
	code/comn/custom         \
	code/comn                \
	buildcfg                 \
	code/comn/ambl           \
	code/comn/fte            \
	code/comn/fte/joins      \
	code/comn/ntlpp          \
	code/comn/autogen/ambl   \
	code/comn/aall           \
	code/stubs/sms           \
	code/stubs/qcft          \
	code/stubs/amx           \
	code/comn/nbase/unix     \
	code/comn/nbase/unixmt   \
	code/comn/nbase/portable \
	code/comn/nbase/base     \
	code/cipr/utils          \
	code/cipr/qrml           \
	code/stubs/li            \
	code/stubs/sck/          \
	code/stubs/l2f/          \
	code/stubs/hals/         \
	code/stubs/smi           \
	code/stubs/nar           \
	code/comn/l2             \
	code/lim                 \
	code/bgp                 \
	code/stubs/l2/unix       \
	code/comn/worker         \
	code/lim                 \
	autogen/comn/ambl        \
	output/$(GMK_TARGETOS)

MS_LD_LIBS = dcsmi dcl2f si
