TOPDIR = ../../../../..
PROGRAM = rw_test
GTEST_ROOT = $(TOPDIR)/third-party/googletest-release-1.8.0/googletest
LIBS = \
       $(GTEST_ROOT)/make/gtest.a \
       $(NULL)
LDFLAGS = -Wl,-rpath $(TOPDIR)/asic/capri/model/capsim-gen/lib -L/usr/local/lib -L$(TOPDIR)/model_sim/build -L$(TOPDIR)/obj `pkg-config --libs grpc++ grpc` -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed -lprotobuf -lpthread -ldl -lhostmem -lzmq
SYSTEM_LIBRARIES = $(LIBS) $(LDFLAGS) -lpthread -lhal -liris -lhal -lperiodic -lp4pdapi -lp4pd -lp4pdtcprx -lp4pdtcptx -lp4pluspd_rxdma -lp4pluspd_txdma -lcapricsr \
				   -lcapri -lslab -lindexer -lht -ltwheel -lbitmap -lprint -lthread -ltrace -lzmq -lpdutils_stub -lhostmem -lfte -lnetwork

LCXXFLAGS = \
            -I.. \
            -I../.. \
            -I$(TOPDIR)/include \
            -I$(TOPDIR)/gen/common_rxdma_actions/include \
            -I$(TOPDIR)/gen/common_txdma_actions/include \
            -I$(TOPDIR)/hal/pd \
            -I$(TOPDIR)/gen/iris/include \
            -I$(TOPDIR)/third-party/spdlog/include \
            -I$(GTEST_ROOT)/include \
            -Wno-sign-compare \
            $(NULL)

SOURCES = \
    rw_tbl_lib_test.cc \
    $(NULL)


include $(TOPDIR)/make/Makefile.inc

GEN_TEST_RESULTS_DIR = $(TOPDIR)/gen/test_results
gtest:
	LD_LIBRARY_PATH=$(TOPDIR)/obj:$(TOPDIR)/asic/capri/model/capsim-gen/lib HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/rw_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/rw_tbl_lib_test.xml"
