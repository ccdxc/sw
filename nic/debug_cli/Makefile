# This Makefile is auto-generated. Changes will be overwritten!
SW_DIR         =  ../..
NIC_DIR        =  $(SW_DIR)/nic
IRIS_CLI_DIR   =  $(NIC_DIR)/gen/iris/cli
GFT_CLI_DIR    =  $(NIC_DIR)/gen/gft/cli
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
SHARED_LIBS    += -L$(NIC_DIR)/obj
ifeq ($(ARCH),aarch64)
SHARED_LIBS    += -L$(TOOLCHAIN_PATH)/aarch64-linux-gnu/usr/lib
endif
SHARED_LIBS    += -lhalproto

default: iris

swig:
	swig -c++ -python -I$(NIC_DIR)/gen/iris/include -I$(NIC_DIR)/hal/pd -o $(IRIS_CLI_DIR)/iris_wrap.cc $(IRIS_CLI_DIR)/iris.i

iris: swig
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -o $(IRIS_CLI_DIR)/_iris.so $(IRIS_CLI_DIR)/iris_wrap.cc $(NIC_DIR)/gen/iris/src/p4pd_debug.cc $(ARCHIVES) $(SHARED_LIBS)

swig_gft:
	swig -c++ -python -I$(NIC_DIR)/gen/gft/include -I$(NIC_DIR)/hal/pd -o $(GFT_CLI_DIR)/gft_wrap.cc $(GFT_CLI_DIR)/gft.i

gft: swig_gft
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -o $(GFT_CLI_DIR)/_gft.so $(GFT_CLI_DIR)/gft_wrap.cc $(NIC_DIR)/gen/gft/src/p4pd_debug.cc $(ARCHIVES) $(SHARED_LIBS)

clean:
	rm -f $(IRIS_CLI_DIR)/_iris.so $(IRIS_CLI_DIR)/iris_wrap.cc $(IRIS_CLI_DIR)/iris.py
	rm -f $(GFT_CLI_DIR)/_gft.so   $(GFT_CLI_DIR)/gft_wrap.cc   $(GFT_CLI_DIR)/gft.py

.PHONY: swig
