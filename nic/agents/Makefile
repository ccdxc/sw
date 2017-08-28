

TOPDIR = ..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

CXX = g++
CPPFLAGS += -I/usr/local/include -std=c++11 -pthread
PROTOC = protoc
C_PROTOC = protoc-c
GRPC_CPP_PLUGIN = grpc_cpp_plugin
GRPC_CPP_PLUGIN_PATH ?= `which $(GRPC_CPP_PLUGIN)`
OBJ_DIR = ../obj
GEN_DIR = ../gen
PROTO_DIR = ./proto
LDFLAGS = -L/usr/local/lib -L../obj/ `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl
INCLUDE_DIRS =  -I../gen/ -I../include \
		-I../include/hal/api/include \
		-Inetwork/ \
		-I../gen/protobuf \
		-I../third-party/spdlog/include

all: 
	$(CXX) $(CPPFLAGS) -I$(GEN_DIR) -I../gen/protobuf -c -o $(OBJ_DIR)/vlan_test_grpc_client.o test/vlan_test_grpc_client.cc
	$(CXX) $(OBJ_DIR)/types.grpc.pb.o $(OBJ_DIR)/types.pb.o $(OBJ_DIR)/net.pb.o $(OBJ_DIR)/net.grpc.pb.o $(OBJ_DIR)/vlan_test_grpc_client.o $(LDFLAGS) -o $(OBJ_DIR)/vlan_test_grpc_client
	$(CXX) $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $(OBJ_DIR)/nw_agent.o network/nw_agent.cc
	$(CXX) $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $(OBJ_DIR)/svc_reg.o network/svc_reg.cc
	$(CXX) $(CPPFLAGS) $(INCLUDE_DIRS) -c -o $(OBJ_DIR)/nw_svc.o network/nw_svc.cc
	$(CXX) $(OBJ_DIR)/types.pb.o $(OBJ_DIR)/net.pb.o $(OBJ_DIR)/net.grpc.pb.o $(OBJ_DIR)/nw_agent.o $(OBJ_DIR)/svc_reg.o $(OBJ_DIR)/nw_svc.o $(LDFLAGS) -lhal -lperiodic -liris -lp4pdapi -lp4pd -lp4pdtcprx -lp4pdtcptx -lp4pluspd_rxdma -lcapri -lcapricsr -lbitmap -lslab -lindexer -lht -ltwheel -ltrace -lbitmap -lthread -lprint -lpdutils -lhostmem -o $(OBJ_DIR)/nw_agent

clean:
	rm -f $(OBJ_DIR)/*.o $(OBJ_DIR)/*.d $(GEN_DIR)/*.cc $(GEN_DIR)/*.h $(OBJ_DIR)/vlan_test_grpc_client
	rm -f $(OBJ_DIR)/nw_agent
