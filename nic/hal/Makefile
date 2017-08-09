LIBDIRS = \
		  pd \
		  $(NULL)
PGMDIRS = $(NULL)
SUBDIRS = $(LIBDIRS) $(PGMDIRS)
TOPDIR = ..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

CXX = g++
CPPFLAGS += -I/usr/local/include -std=c++11 -pthread -g -c -Wall -Werror
OBJ_DIR = ../obj
GEN_DIR = ../gen
GTEST_DIR = test
GTEST_ROOT = ../third-party/googletest-release-1.8.0/googletest
GEN_TEST_RESULTS_DIR = ../gen/test_results

INC_DIRS = -I. -I$(GEN_DIR) -I$(GEN_DIR)/protobuf -Isvc -Isrc -I../utils/ -Ipd/common -I../include -Ipd \
		-I$(GTEST_ROOT)/include -I../third-party/spdlog/include
LCXXFLAGS = $(INC_DIRS)
LDFLAGS = -L/usr/local/lib -L../model_sim/build -L../obj/ `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lhalproto -lprotobuf -lpthread -ldl -lzmq

LDFLAGS +=	-lhal \
			-liris \
			-lperiodic \
			-lp4pdapi \
			-lp4pd \
			-lp4pdtcprx \
			-lp4pdtcptx \
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
			-lpdcommon

SRC_FILES :=	hal.cc \
				hal_mem.cc \
				main.cc \
				$(NULL)

SHARED_OBJS :=	$(OBJ_DIR)/libhal.so \
				$(OBJ_DIR)/hal.o \
				$(OBJ_DIR)/hal_mem.o \
				$(OBJ_DIR)/hal_pd.o \
				$(OBJ_DIR)/utils.o \
				$(OBJ_DIR)/types.pb.o \
				$(OBJ_DIR)/tenant.pb.o \
				$(OBJ_DIR)/tenant.o \
				$(OBJ_DIR)/interface.pb.o \
				$(OBJ_DIR)/interface.o \
				$(OBJ_DIR)/l2segment.pb.o \
				$(OBJ_DIR)/l2segment.o \
				$(OBJ_DIR)/session.pb.o \
				$(OBJ_DIR)/session.o \
				$(OBJ_DIR)/nwsec.pb.o \
				$(OBJ_DIR)/nwsec.o \
				$(OBJ_DIR)/qos.pb.o \
				$(OBJ_DIR)/qos.o \
				$(OBJ_DIR)/interface_api.o \
				$(OBJ_DIR)/l2segment_api.o \
				$(OBJ_DIR)/endpoint_api.o \
				$(OBJ_DIR)/nwsec_api.o \
				$(OBJ_DIR)/qos_api.o \
				$(OBJ_DIR)/endpoint.pb.o \
				$(OBJ_DIR)/endpoint.o \
				$(OBJ_DIR)/l4lb.pb.o \
				$(OBJ_DIR)/tls_proxy_cb.pb.o \
				$(OBJ_DIR)/tlscb.o \
				$(OBJ_DIR)/tcp_proxy_cb.pb.o \
				$(OBJ_DIR)/tcpcb.o \
	

HAL_OBJS := $(OBJ_DIR)/main.o 

#OBJ_FILES := $(patsubst %.cc, %.o, $(SRC_FILES))
OBJ_FILES := $(basename $(SRC_FILES))
OBJ_FILES := $(OBJ_FILES:%=$(GENDIR)/%.o)

$(OBJ_DIR)/%.o: %.cc
	@echo Compiling $@
	$(CXX) $(CPPFLAGS) $(INC_DIRS) -fpic $< -o $(OBJ_DIR)/$(shell basename $@)

all: $(OBJ_FILES)
	@#@echo $(OBJ_FILES)
	@#$(CXX) $(CPPFLAGS) $(INC_DIRS) -o $(OBJ_DIR)/main.o main.cc
	$(CXX) -g -shared -o $(SHARED_OBJS)
	$(CXX) $(HAL_OBJS) $(LDFLAGS) -lpdutils -o $(OBJ_DIR)/hal
	$(CXX) $(HAL_OBJS) $(LDFLAGS) -lpdutils_stub -o $(OBJ_DIR)/hal_stub

clean:
	rm -rf $(OBJ_DIR)/hal.* $(OBJ_DIR)/hal_mem.* $(OBJ_DIR)/main.* $(OBJ_DIR)/*.so $(OBJ_DIR)/hal $(OBJ_DIR)/hal_stub
