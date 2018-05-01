# This Makefile is auto-generated. Changes will be overwritten!
SW_DIR         =  ../../
NIC_DIR        =  $(SW_DIR)/nic
PGM            := iris
CLI_DIR        =  $(NIC_DIR)/gen/$(PGM)/cli
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

default: $(PGM)

swig:
	swig -c++ -python -I$(NIC_DIR)/gen/$(PGM)/include -I$(NIC_DIR)/hal/pd -o $(CLI_DIR)/$(PGM)_wrap.cc $(CLI_DIR)/$(PGM).i

$(PGM): swig
    ifeq ($(PGM),$(filter $(PGM),iris gft))
		$(CXX) $(CPPFLAGS) $(INC_DIRS) -o $(CLI_DIR)/_$(PGM).so $(CLI_DIR)/$(PGM)_wrap.cc $(NIC_DIR)/gen/$(PGM)/src/p4pd_debug.cc        $(ARCHIVES) $(SHARED_LIBS)
    else
		$(CXX) $(CPPFLAGS) $(INC_DIRS) -o $(CLI_DIR)/_$(PGM).so $(CLI_DIR)/$(PGM)_wrap.cc $(NIC_DIR)/gen/$(PGM)/src/$(PGM)_p4pd_debug.cc $(ARCHIVES) $(SHARED_LIBS)
    endif

clean:
	rm -f $(CLI_DIR)/_$(PGM).so $(CLI_DIR)/$(PGM)_wrap.cc $(CLI_DIR)/$(PGM).py

.PHONY: swig
