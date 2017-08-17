TOPDIR = ../../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

OBJ_DIR = $(TOPDIR)/obj
GEN_DIR = $(TOPDIR)/gen

GTEST_ROOT = $(TOPDIR)/third-party/googletest-release-1.8.0/googletest
INC_DIRS = -I. -I$(GEN_DIR) -I$(GEN_DIR)/protobuf -I$(GEN_DIR)/iris/include -I$(TOPDIR)/hal/svc -I$(TOPDIR)/hal/src -I$(TOPDIR)/utils/ -I$(TOPDIR)/hal/pd/common -I$(TOPDIR)/include -I$(TOPDIR)/hal/pd \
		-I$(GTEST_ROOT)/include -I$(TOPDIR)/third-party/spdlog/include \
		-I$(TOPDIR)/p4/nw/include

LDFLAGS = -L/usr/local/lib -L$(TOPDIR)/model_sim/build -L$(TOPDIR)/obj `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl -lzmq

LCLEANFILES = $(OBJ_DIR)/lif_test $(OBJ_DIR)/uplinkif_test $(OBJ_DIR)/uplinkpc_test $(OBJ_DIR)/enicif_test $(OBJ_DIR)/session_test

CUS_SOURCES = $(wildcard *.cc)
CUS_PFXES := $(basename $(CUS_SOURCES))
CUS_BINARIES := $(CUS_PFXES:%=$(OBJ_DIR)/%)

all: $(CUS_BINARIES)

quiet_cmd_cuslink = LINK	$(basename $<)
cmd_cuslink = $(CXX) -std=c++11 $(INC_DIRS) -Wno-sign-compare -g $< $(GTEST_ROOT)/make/gtest.a $(LDFLAGS) -lpthread -lhal -liris -lperiodic -lp4pdapi -lp4pd -lp4pdtcprx -lp4pdtcptx -lcapricsr -lcapri -lslab -lindexer -lht -ltwheel -lbitmap -lprint -lthread -ltrace -lzmq -lpdutils_stub -o $@

$(OBJ_DIR)/%: %.cc
	$(call cmd,cuslink)

GEN_TEST_RESULTS_DIR = $(TOPDIR)/gen/test_results
gtest:
	CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$(TOPDIR)/obj WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/lif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/lif_test.xml"
	CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$(TOPDIR)/obj WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/uplinkif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/uplinkif_test.xml"
	CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$(TOPDIR)/obj WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/uplinkpc_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/uplinkpc_test.xml"
	CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$(TOPDIR)/obj WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/enicif_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/enicif_test.xml"
	CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$(TOPDIR)/obj WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/buf_pool_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/buf_pool_test.xml"
	CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$(TOPDIR)/obj WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/queue_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/queue_test.xml"
	CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$(TOPDIR)/obj WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/policer_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/policer_test.xml"
	CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$(TOPDIR)/obj WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/session_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/session_test.xml"
	CAPRI_MOCK_MODE=1 LD_LIBRARY_PATH=$(TOPDIR)/obj WS_TOP=$(TOPDIR)/../../.. HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/acl_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/acl_test.xml"
