# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

SW_DIR         =  ../..
NIC_DIR        =  $(SW_DIR)/nic

TOOLCHAIN_ROOT =  /tool/toolchain/aarch64
TOOLCHAIN_VER  =  1.1
TOOLCHAIN_PATH =  $(TOOLCHAIN_ROOT)-$(TOOLCHAIN_VER)

ifneq ($(ARCH),aarch64)
    ARCH = x86_64
endif

ifeq ($(ARCH),aarch64)
CXX            =  aarch64-linux-gnu-g++
else
CXX            =  g++
endif

CPPFLAGS       =  -shared -fPIC

INC_DIRS       =  -I$(SW_DIR)
INC_DIRS       += -I$(NIC_DIR)/hal/third-party/grpc/include
INC_DIRS       += -I$(NIC_DIR)/hal/third-party/google/include
INC_DIRS	   += -I/usr/include/python3.6m
INC_DIRS	   += -I/usr/include/python3.4m
ifeq ($(ARCH),aarch64)
INC_DIRS       += -I$(TOOLCHAIN_PATH)/aarch64-linux-gnu/usr/include
endif

ARCHIVES       =  -Wl,--allow-multiple-definition
ARCHIVES       += $(NIC_DIR)/hal/third-party/grpc/$(ARCH)/lib/libgrpc*.a -Wl,--no-whole-archive

SHARED_LIBS    =  $(NIC_DIR)/hal/third-party/google/$(ARCH)/lib/libprotobuf.so.14
ifeq ($(ARCH),aarch64)
SHARED_LIBS    += -L$(TOOLCHAIN_PATH)/aarch64-linux-gnu/usr/lib
SHARED_LIBS    += -L$(NIC_DIR)/gen/aarch64/lib
else
SHARED_LIBS    += -L$(NIC_DIR)/gen/x86_64/lib
endif
SHARED_LIBS    += -lhalproto

PGM := iris

ifeq ($(PGM),cli)
CLI_DIR = $(NIC_DIR)/debug_cli/cli
else
CLI_DIR = $(NIC_DIR)/gen/$(PGM)/cli
endif

ifeq ($(PGM),cli)
SOURCES = $(CLI_DIR)/$(PGM)_wrap.cc $(NIC_DIR)/debug_cli/src/cli.cc
else ifeq ($(PGM),$(filter $(PGM),iris gft))
SOURCES = $(CLI_DIR)/$(PGM)_wrap.cc $(NIC_DIR)/gen/$(PGM)/src/p4pd_cli_backend.cc
else
SOURCES = $(CLI_DIR)/$(PGM)_wrap.cc $(NIC_DIR)/gen/$(PGM)/src/$(PGM)_p4pd_cli_backend.cc $(NIC_DIR)/gen/$(PGM)/src/$(PGM)_p4plus_entry_packing.cc
endif

default: $(PGM)

$(CLI_DIR)/.swig: $(CLI_DIR)/$(PGM).i
	@touch $@
    ifeq ($(PGM),cli)
	swig -c++ -python -o $(CLI_DIR)/$(PGM)_wrap.cc $(CLI_DIR)/$(PGM).i
    else
	swig -c++ -python -I$(NIC_DIR)/gen/$(PGM)/include -I$(NIC_DIR)/hal/pd -o $(CLI_DIR)/$(PGM)_wrap.cc $(CLI_DIR)/$(PGM).i
    endif

swig: $(CLI_DIR)/.swig

$(CLI_DIR)/.dir: $(SOURCES)
	@touch $@
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -o $(CLI_DIR)/_$(PGM).so $(SOURCES) $(ARCHIVES) $(SHARED_LIBS)

sources: $(CLI_DIR)/.dir

$(PGM):
	$(MAKE) swig
	$(MAKE) sources

clean:
	rm -f $(CLI_DIR)/_$(PGM).so $(CLI_DIR)/$(PGM)_wrap.cc $(CLI_DIR)/$(PGM).py $(CLI_DIR)/.swig $(CLI_DIR)/.dir

.PHONY: swig cli
