
TOPDIR = ../../../..
OBJ_DIR = ./obj

CXX = g++
CPPFLAGS += -I/usr/local/include -std=c++11 -pthread -g -c -Wall -Werror -O0
CPPFLAGS += -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.d

INC_DIRS = -I. -I$(TOPDIR) -I$(TOPDIR)/nic/sdk -I$(TOPDIR)/nic/sdk/dpdk/build/include/ \
		-I$(TOPDIR)/nic/build/x86_64/athena/ -I$(TOPDIR)/nic/hal/third-party/spdlog/include
GMK_TARGETOS = x86_64
MODE = debug
LDFLAGS = -L$(TOPDIR)/nic/build/x86_64/athena/lib/ -Wl,-rpath=/sw/nic/build/x86_64/athena/lib/ -L/usr/local/lib64 -L/lib64 \
		-L$(TOPDIR)/nic/third-party/metaswitch/output/$(GMK_TARGETOS)/$(MODE)
LDLIBS += -lpal -lpdsframework -lpdscore -lpdslearn -lpdsapi -lpdsapi_impl -lsdkftl \
		-ldpdksim -ldpdk -lthread -ltrace -llogger -lmemhash -lsltcam \
		-levent_thread -lslhash -lp4pd_athena -lp4pd_athena_rxdma \
		-lp4pd_athena_txdma -lutils -llist -lslab -lshmmgr -lmmgr \
		-lsdkpal -lsdkfru -lht -lindexer -lperiodic -ltwheel -ldirectmap \
		-lsldirectmap -lhash -lhbmhash -ltcam -ltimerfd -lcatalog \
		-ldevice -lsdkplatformutils -lsdkcapri -lsdkp4loader -lsdkasicrw \
		-llif_mgr -lsdkring -lsysmon -lsensor -lmodel_client -lsdkp4 \
		-lsdkp4utils -lsdkcapri_asicrw_if -lpdsapi_capri_impl \
		-lsdkxcvrdriver -lsdkasicpd -lbm_allocator -lsdklinkmgr \
		-lsdklinkmgrcsr -lnicmgr_athena -lmnet -levutils \
		-lpciemgr_if -lpciemgr -lpciemgrutils -lpciehdevices \
		-lpcietlp -lcfgspace -lintrutils -lmisc \
		-lev \
		-lmpuobj -lcapisa -lisa -lsknobs -lsdkcapri_csrint -lAAPL \
		-lpthread -lz -lrt -lJudy -ldl -lzmq -ledit -lncurses -ltinfo \
		-lstdc++ -lm -lgcc -lc

SRC_FILES := trace.cc \
		fte_athena.cc \
		fte_flow_athena.cc \
		main.cc \
		$(NULL)

OBJECTS := $(basename $(SRC_FILES))
OBJECTS := $(OBJECTS:%=$(OBJ_DIR)/%.o)
DEPENDS = $(OBJECTS:.o=.d)

ATH_APP_OBJS := $(OBJ_DIR)/main.o $(OBJ_DIR)/trace.o $(OBJ_DIR)/fte_athena.o $(OBJ_DIR)/fte_flow_athena.o

OBJ_FILES := $(basename $(SRC_FILES))
OBJ_FILES := $(OBJ_FILES:%=$(OBJ_DIR)/%.o)

$(OBJ_DIR)/%.o: %.cc
	@mkdir -p $(OBJ_DIR)
	@echo Compiling $@
	@echo **NOTE: Fix for __FNAME__ related issues: Replace __FNAME__ by __FUNCTION__ in logger.hpp**
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -fpic $< -o $(OBJ_DIR)/$(shell basename $@)

all: $(OBJ_FILES)
	$(CXX) $(ATH_APP_OBJS) $(LDFLAGS) $(LDLIBS) -o $(OBJ_DIR)/athena_app

clean:
	rm -rf $(OBJ_DIR)

-include $(DEPENDS)
