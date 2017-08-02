
LIBDIRS = $(NULL)
PGMDIRS = $(NULL)
SUBDIRS = $(LIBDIRS) $(PGMDIRS)
TOPDIR = ../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

CXX = g++
CPPFLAGS += -I/usr/local/include -I../../include -I../../third-party/spdlog/include -std=c++11 -pthread -g
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
LDFLAGS = -L/usr/local/lib -L../../obj/ `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lproto -lprotobuf -lpthread -ldl

all:
	$(CXX) $(CPPFLAGS) -I$(GEN_DIR) -I$(GEN_DIR)/protobuf -c -o $(OBJ_DIR)/hal_test.o hal_test.cc
	$(CXX) $(CPPFLAGS) -I$(GEN_DIR) -fPIC -c -o $(OBJ_DIR)/hal_mock.o hal_mock.cc
	$(CXX) -g -shared -o $(OBJ_DIR)/libhalmock.so $(OBJ_DIR)/hal_mock.o
	@#$(CXX) $(OBJ_DIR)/types.grpc.pb.o $(OBJ_DIR)/types.pb.o $(OBJ_DIR)/tenant.pb.o $(OBJ_DIR)/tenant.grpc.pb.o $(OBJ_DIR)/l2segment.pb.o $(OBJ_DIR)/l2segment.grpc.pb.o $(OBJ_DIR)/interface.pb.o $(OBJ_DIR)/interface.grpc.pb.o $(OBJ_DIR)/endpoint.pb.o $(OBJ_DIR)/endpoint.grpc.pb.o $(OBJ_DIR)/session.pb.o $(OBJ_DIR)/session.grpc.pb.o $(OBJ_DIR)/qos.pb.o $(OBJ_DIR)/qos.grpc.pb.o $(OBJ_DIR)/hal_test.o $(LDFLAGS) -o $(OBJ_DIR)/hal_test
	$(CXX) $(OBJ_DIR)/hal_test.o $(LDFLAGS) -o $(OBJ_DIR)/hal_test
	@# $(CXX) $(OBJ_DIR)/types.grpc.pb.o $(OBJ_DIR)/types.pb.o $(OBJ_DIR)/tenant.pb.o $(OBJ_DIR)/tenant.grpc.pb.o $(OBJ_DIR)/l2segment.pb.o $(OBJ_DIR)/l2segment.grpc.pb.o $(OBJ_DIR)/interface.pb.o $(OBJ_DIR)/interface.grpc.pb.o $(OBJ_DIR)/endpoint.pb.o $(OBJ_DIR)/endpoint.grpc.pb.o $(OBJ_DIR)/session.pb.o $(OBJ_DIR)/session.grpc.pb.o $(OBJ_DIR)/hal_test.o $(LDFLAGS) -o $(OBJ_DIR)/hal_test

clean:
	@rm -rf $(OBJ_DIR)/hal_test
