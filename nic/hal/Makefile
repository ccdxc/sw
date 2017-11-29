
LIBDIRS = \
	pd \
	plugins \
	$(NULL)
PGMDIRS = $(NULL)
SUBDIRS = $(LIBDIRS) $(PGMDIRS)
TOPDIR = ..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

CXX = g++
CPPFLAGS += -I/usr/local/include -std=c++11 -pthread -g -c -Wall -Werror -O0
CPPFLAGS += -MT $@ -MMD -MP -MF $(GENDIR)/$*.d
OBJ_DIR = ../obj
GEN_DIR = ../gen
GTEST_DIR = test
GTEST_ROOT = ../third-party/googletest-release-1.8.0/googletest
GEN_TEST_RESULTS_DIR = ../gen/test_results
LIBLKL_ROOT = ../third-party/lkl/export/

INC_DIRS = -I../ -I. -I$(GEN_DIR) -I$(GEN_DIR)/iris/include -I$(GEN_DIR)/protobuf -Isvc -Isrc -I../utils/ -Ipd/common -I../include -Ipd -I../p4/nw/include \
		-I$(GTEST_ROOT)/include -I../third-party/spdlog/include -Ilkl -I../asm/cpu-p4+/include
LCXXFLAGS = $(INC_DIRS)
LDFLAGS = -Wl,-rpath $(TOPDIR)/asic/capri/model/capsim-gen/lib -L/usr/local/lib -L../model_sim/build -L../obj/ `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lhalproto -lprotobuf -lpthread -ldl -lzmq

LDFLAGS +=	-lhal \
			-liris \
			-lfte \
			-lnetwork \
			-lperiodic \
			-lp4pdapi \
			-lp4pd \
			-lp4pluspd_rxdma \
			-lp4pluspd_txdma \
			-lcapricsr \
			-lcapri \
			-lslab \
			-lindexer \
			-lht \
			-ltwheel \
			-lbitmap \
			-ltrace \
			-lprint \
			-lthread \
			-lzmq \
			-lsvc \
			-lpdcommon \
			-lhostmem \
			-lproxyplugin \
			-lapp_redir \
			-llkl_api \

LDFLAGS += -L$(LIBLKL_ROOT)/bin -llkl
LDLIBS = -llkl

SRC_FILES :=	hal.cc \
				hal_state.cc \
				hal_cfg.cc \
				main.cc \
				$(NULL)

OBJECTS := $(basename $(SRC_FILES))
OBJECTS := $(OBJECTS:%=$(GENDIR)/%.o)
DEPENDS = $(OBJECTS:.o=.d)

SHARED_OBJS :=	$(OBJ_DIR)/libhal.so \
				$(OBJ_DIR)/hal.o \
				$(OBJ_DIR)/hal_state.o \
				$(OBJ_DIR)/hal_cfg.o \
				$(OBJ_DIR)/hal_pd.o \
				$(OBJ_DIR)/utils.o \
				$(OBJ_DIR)/if_utils.o \
				$(OBJ_DIR)/types.pb.o \
				$(OBJ_DIR)/vrf.pb.o \
				$(OBJ_DIR)/vrf.o \
				$(OBJ_DIR)/interface.pb.o \
				$(OBJ_DIR)/internal.pb.o \
				$(OBJ_DIR)/rdma.pb.o \
				$(OBJ_DIR)/interface.o \
				$(OBJ_DIR)/lif.o \
				$(OBJ_DIR)/internal.o \
				$(OBJ_DIR)/network.o \
				$(OBJ_DIR)/nw.pb.o \
				$(OBJ_DIR)/l2segment.pb.o \
				$(OBJ_DIR)/l2segment.o \
				$(OBJ_DIR)/session.pb.o \
				$(OBJ_DIR)/session.o \
				$(OBJ_DIR)/nwsec.pb.o \
				$(OBJ_DIR)/nwsec.o \
				$(OBJ_DIR)/nwsec_group.o \
				$(OBJ_DIR)/qos.pb.o \
				$(OBJ_DIR)/qos.o \
				$(OBJ_DIR)/acl.pb.o \
				$(OBJ_DIR)/acl.o \
				$(OBJ_DIR)/interface_api.o \
				$(OBJ_DIR)/l2segment_api.o \
				$(OBJ_DIR)/oif_list_mgr.o \
				$(OBJ_DIR)/endpoint_api.o \
				$(OBJ_DIR)/nwsec_api.o \
				$(OBJ_DIR)/qos_api.o \
				$(OBJ_DIR)/acl_api.o \
				$(OBJ_DIR)/endpoint.pb.o \
				$(OBJ_DIR)/endpoint.o \
				$(OBJ_DIR)/l4lb.pb.o \
				$(OBJ_DIR)/bm_allocator.o \
				$(OBJ_DIR)/lif_manager_base.o \
				$(OBJ_DIR)/lif_manager.o \
				$(OBJ_DIR)/tls_proxy_cb.pb.o \
				$(OBJ_DIR)/tlscb.o \
				$(OBJ_DIR)/tcp_proxy_cb.pb.o \
				$(OBJ_DIR)/tcpcb.o \
				$(OBJ_DIR)/descriptor_aol.pb.o \
				$(OBJ_DIR)/descriptor_aol.o \
				$(OBJ_DIR)/wring.pb.o \
				$(OBJ_DIR)/wring.o \
				$(OBJ_DIR)/proxy.pb.o \
				$(OBJ_DIR)/proxy.o \
				$(OBJ_DIR)/rdma.o \
				$(OBJ_DIR)/telemetry.pb.o \
				$(OBJ_DIR)/telemetry.o \
				$(OBJ_DIR)/crypto_keys.o \
				$(OBJ_DIR)/crypto_keys.pb.o \
				$(OBJ_DIR)/ipseccb.pb.o \
				$(OBJ_DIR)/ipseccb.o \
				$(OBJ_DIR)/l4lb.o \
				$(OBJ_DIR)/cpucb.pb.o \
				$(OBJ_DIR)/cpucb.o \
				$(OBJ_DIR)/rawrcb.pb.o \
				$(OBJ_DIR)/rawrcb.o \
				$(OBJ_DIR)/rawccb.pb.o \
				$(OBJ_DIR)/rawccb.o \
				$(OBJ_DIR)/system.o \


HAL_OBJS := $(OBJ_DIR)/main.o 

#OBJ_FILES := $(patsubst %.cc, %.o, $(SRC_FILES))
OBJ_FILES := $(basename $(SRC_FILES))
OBJ_FILES := $(OBJ_FILES:%=$(GENDIR)/%.o)

$(OBJ_DIR)/%.o: %.cc
	@echo Compiling $@
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -fpic $< -o $(OBJ_DIR)/$(shell basename $@) $(GCOV)

all: $(OBJ_FILES)
	@#@echo $(OBJ_FILES)
	@#$(CXX) $(CPPFLAGS) $(INC_DIRS) -o $(OBJ_DIR)/main.o main.cc
	$(CXX) -g -shared -o $(SHARED_OBJS) $(GCOV)
	$(CXX) $(HAL_OBJS) $(LDFLAGS) -lpdutils -o $(OBJ_DIR)/hal $(GCOV) -lgcov
	$(CXX) $(HAL_OBJS) $(LDFLAGS) -lpdutils -o $(OBJ_DIR)/hal_stub $(GCOV) -lgcov

clean:
	rm -rf $(OBJ_DIR)/hal.* $(OBJ_DIR)/hal_state.* $(OBJ_DIR)/hal_cfg.* $(OBJ_DIR)/main.* $(OBJ_DIR)/*.so $(OBJ_DIR)/hal $(OBJ_DIR)/hal_stub

-include $(DEPENDS)
