
TOPDIR = ../../../..

OBJ_DIR = ./obj

export ARCH ?= x86_64
export ASIC ?= capri
CXX = /tool/toolchain/aarch64-1.1/bin/aarch64-linux-gnu-g++
CPPFLAGS += -I/usr/local/include -std=c++11 -pthread -g -c -Wall -Werror -O0
CPPFLAGS += -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.d -D__FNAME__=__FUNCTION__ -DRTE_FORCE_INTRINSICS
ifeq ($(ARCH), x86_64)
CXX = g++
CPPFLAGS  += -march=native
endif

INC_DIRS = -I. -I$(TOPDIR) -I$(TOPDIR)/nic/sdk -I$(TOPDIR)/nic/sdk/dpdk/build/include/ \
		-I$(TOPDIR)/nic/build/$(ARCH)/athena/${ASIC} -I$(TOPDIR)/nic/hal/third-party/spdlog/include \
		-I$(TOPDIR)/nic/sdk/third-party/googletest-release-1.8.0/googletest/include \
                -I$(TOPDIR)/nic/sdk/third-party/boost/include \
                -I$(TOPDIR)/nic/hal/third-party/google/include \
                -I$(TOPDIR)/nic/build/$(ARCH)/athena/${ASIC}/gen/proto \
				-I$(TOPDIR)/nic/sdk/third-party/libev/include

MODE = debug
LDFLAGS = -L$(TOPDIR)/nic/build/$(ARCH)/athena/${ASIC}/lib/ -Wl,-rpath=$(TOPDIR)/nic/build/$(ARCH)/athena/${ASIC}/lib/ \
		-L$(TOPDIR)/nic/third-party/metaswitch/output/$(ARCH)/$(MODE)

ifeq ($(ARCH), x86_64)
LDFLAGS += -L/usr/local/lib64 -L/lib64 -L$(TOPDIR)/nic/sdk/third-party/googletest-release-1.8.0/googletest/make -l:gtest.a
else
LDFLAGS += -L/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/usr/lib \
           -L/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/lib \
           -L/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/usr/lib/aarch64-linux-gnu
endif

LDLIBS += -lpal -lpdsframework -lpdscore -lpdslearn -lpdsapi -lpdsapi_impl -lsdkftl \
		-lpdsagent_athena -lprotobuf\
		-lgrpc++_reflection -lgrpc++ -lgrpc_unsecure -lgrpc++_unsecure \
		-ldpdk -lthread -ltrace -llogger -lmemhash -lsltcam -leth_p4plus \
		-levent_thread -lslhash -lpenmetrics -lp4pd_athena -lp4pd_athena_rxdma \
		-lp4pd_athena_txdma -lutils -llist -lslab -lshmmgr -lmmgr \
		-lsdkpal -lsdkfru -lht -lindexer -lperiodic -ltwheel -ldirectmap \
		-lsldirectmap -lhash -lhbmhash -ltcam -ltimerfd -lcatalog \
		-ldevice -lsdkplatformutils -lsdkcapri -lsdkp4loader -lsdkasicrw \
		-llif_mgr -lsdkring -lsysmon -lsensor -lsdkp4 \
		-lsdkp4utils -lsdk_asicrw_if -lpdsapi_asic_impl \
		-lsdkxcvrdriver -lsdkasicpd -lsdkasiccmn -lkvstore_lmdb -llmdb -lbm_allocator -lsdklinkmgr \
		-lsdklinkmgrcsr -lnicmgr_athena -lmnet -levutils \
		-lpciemgr_if -lpciemgr -lpciemgrutils -lpciehdevices \
		-lpcietlp -lcfgspace -lintrutils -lmisc -lpdsupg_nicmgr_stubs \
		-lev -lrte_indexer \
		-lmpuobj -lcapisa -lisa -lsknobs -lsdkcapri_csrint -lAAPL \
		-lpthread -lz -lrt -lJudy -ldl -lzmq -ledit -lncurses \
		-lstdc++ -lm -lgcc -lc

ifeq ($(ARCH), x86_64)
LDLIBS += -lmodel_client -ldpdksim
endif
SRC_FILES := app_test_utils.cc \
		fte_athena.cc \
		fte_flow_athena.cc \
		script_exec.cc \
		script_parser.cc \
		session_aging.cc \
		conntrack_aging.cc \
		combined_aging.cc \
		athena_app_server.cc \
		zmq_wait.cc \
		json_parser.cc \
		main.cc \
		$(NULL)

OBJECTS := $(basename $(SRC_FILES))
OBJECTS := $(OBJECTS:%=$(OBJ_DIR)/%.o)
DEPENDS = $(OBJECTS:.o=.d)

ATH_APP_OBJS := $(OBJ_DIR)/main.o $(OBJ_DIR)/fte_athena.o $(OBJ_DIR)/fte_flow_athena.o \
		$(OBJ_DIR)/app_test_utils.o $(OBJ_DIR)/script_exec.o $(OBJ_DIR)/script_parser.o \
		$(OBJ_DIR)/session_aging.o $(OBJ_DIR)/conntrack_aging.o \
                $(OBJ_DIR)/combined_aging.o $(OBJ_DIR)/athena_app_server.o $(OBJ_DIR)/zmq_wait.o \
                $(OBJ_DIR)/json_parser.o

OBJ_FILES := $(basename $(SRC_FILES))
OBJ_FILES := $(OBJ_FILES:%=$(OBJ_DIR)/%.o)

$(OBJ_DIR)/%.o: %.cc
	@mkdir -p $(OBJ_DIR)
	@echo Compiling $@
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -fpic $< -o $(OBJ_DIR)/$(shell basename $@)

all: $(OBJ_FILES)
	$(CXX) $(ATH_APP_OBJS) $(LDFLAGS) $(LDLIBS) -o $(OBJ_DIR)/athena_app

clean:
	rm -rf $(OBJ_DIR)

-include $(DEPENDS)
