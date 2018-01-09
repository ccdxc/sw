
TOPDIR = ../../../../..
PROGRAM = directmap_test
GTEST_ROOT = $(TOPDIR)/third-party/googletest-release-1.8.0/googletest
LIBS = \
	   $(GTEST_ROOT)/make/gtest.a \
	   $(NULL)
SYSTEM_LIBRARIES = -Wl,-rpath $(TOPDIR)/asic/capri/model/capsim-gen/lib -lpdutils $(LIBS) -Wl,-rpath $(TOPDIR)/asic/capri/model/capsim-gen/lib -L$(TOPDIR)/model_sim/build -L$(TOPDIR)/obj -lpthread -lmodelclient -lzmq -lthread -lcapri -lhalmock -lcapricsr -lp4pd -lp4pluspd_rxdma -lp4pluspd_txdma -lindexer -ltrace -lbitmap

LCXXFLAGS = \
			-I.. \
			-I$(TOPDIR)/include \
			-I$(TOPDIR)/hal/pd \
			-I$(TOPDIR)/gen/iris/include \
			-I$(TOPDIR)/third-party/spdlog/include \
			-I$(GTEST_ROOT)/include \
			-Wno-sign-compare \
			$(NULL)

SOURCES = \
	directmap_test.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc

GEN_TEST_RESULTS_DIR = $(TOPDIR)/gen/test_results
gtest:
	LD_PRELOAD=$(TOPDIR)/obj/libp4pd_mock.so LD_LIBRARY_PATH=$(TOPDIR)/obj:$(TOPDIR)/asic/capri/model/capsim-gen/lib HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/directmap_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/directmap_test.xml" 
