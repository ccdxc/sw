TOPDIR = ../../../../../..

OBJ_DIR = ./obj

export ARCH ?= x86_64
CXX = /tool/toolchain/aarch64-1.1/bin/aarch64-linux-gnu-gcc
CPPFLAGS += -I/usr/local/include -pthread -g -c -Wall -Werror -O0
CPPFLAGS += -MT $@ -MMD -MP -MF $(OBJ_DIR)/$*.d -D__FNAME__=__FUNCTION__ -DRTE_FORCE_INTRINSICS
ifeq ($(ARCH), x86_64)
CXX = gcc
CPPFLAGS  += -march=native
endif

INC_DIRS = -I. -I$(TOPDIR)/nic/apollo/api/include/athena

MODE = debug
LDFLAGS = -L$(TOPDIR)/nic/build/$(ARCH)/athena/lib/ -Wl,-rpath=$(TOPDIR)/nic/build/$(ARCH)/athena/lib/ \

ifeq ($(ARCH), x86_64)
LDFLAGS += -L/usr/local/lib64 -L/lib64
else
LDFLAGS += -L/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/usr/lib \
           -L/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/lib \
           -L/tool/toolchain/aarch64-1.1/aarch64-linux-gnu/usr/lib/aarch64-linux-gnu
endif

LDLIBS += -lpal -lpdsframework -lpdscore -lpdslearn -lpdsapi -lpdsapi_impl -lsdkftl \
                -ldpdk -lthread -ltrace -llogger -lmemhash -lsltcam -leth_p4plus \
                -levent_thread -lslhash -lpenmetrics -lp4pd_athena -lp4pd_athena_rxdma \
                -lp4pd_athena_txdma -lutils -llist -lslab -lshmmgr -lmmgr \
                -lsdkpal -lsdkfru -lht -lindexer -lperiodic -ltwheel -ldirectmap \
                -lsldirectmap -lhash -lhbmhash -ltcam -ltimerfd -lcatalog \
                -ldevice -lsdkplatformutils -lsdkcapri -lsdkp4loader -lsdkasicrw \
                -llif_mgr -lsdkring -lsysmon -lsensor -lsdkp4 \
                -lsdkp4utils -lsdkcapri_asicrw_if -lpdsapi_asic_impl \
                -lsdkxcvrdriver -lsdkasicpd -lsdkasiccmn -lkvstore_lmdb -llmdb -lbm_allocator -lsdklinkmgr \
                -lsdklinkmgrcsr -lnicmgr_athena -lmnet -levutils \
                -lpciemgr_if -lpciemgr -lpciemgrutils -lpciehdevices \
                -lpcietlp -lcfgspace -lintrutils -lmisc -lpdsnicmgr_upgrade_stubs\
                -lev \
                -lmpuobj -lcapisa -lisa -lsknobs -lsdkcapri_csrint -lAAPL \
                -lpthread -lz -lrt -lJudy -ldl -lzmq -ledit -lncurses \
                -lstdc++ -lm -lgcc -lc


SRC_FILES := main.c \
                $(NULL)
OBJECTS := $(basename $(SRC_FILES))
OBJECTS := $(OBJECTS:%=$(OBJ_DIR)/%.o)
DEPENDS = $(OBJECTS:.o=.d)

ATH_APP_OBJS := $(OBJ_DIR)/main.o 

OBJ_FILES := $(basename $(SRC_FILES))
OBJ_FILES := $(OBJ_FILES:%=$(OBJ_DIR)/%.o)

$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(OBJ_DIR)
	@echo Compiling $@
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -fpic $< -o $(OBJ_DIR)/$(shell basename $@)

all: $(OBJ_FILES)
	$(CXX) $(ATH_APP_OBJS) $(LDFLAGS) $(LDLIBS) -o athena_cps

clean:  
	rm -rf $(OBJ_DIR)

-include $(DEPENDS)


