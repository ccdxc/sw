TOPDIR = ../..
PROGRAM = fte_test
OBJ_DIR = $(TOPDIR)/obj
GTEST_ROOT = $(TOPDIR)/third-party/googletest-release-1.8.0/googletest

OBJS = $(OBJ_DIR)/fte.o

SYSTEM_LIBRARIES = -lpthread -lht -ltrace -lprint -Wl,--no-as-needed -lgrpc++_reflection -Wl,--as-needed  -lhalproto -lprotobuf $(OBJS) -ltrace $(GTEST_ROOT)/make/gtest.a

LCXXFLAGS = \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)/hal/src \
	-I$(TOPDIR)/hal/svc \
	-I$(TOPDIR)/gen/protobuf \
	-I$(TOPDIR)/gen/iris/include \
	-I$(TOPDIR)/p4/nw/include \
	-I$(TOPDIR)/third-party/spdlog/include \
	-I$(GTEST_ROOT)/include \
	-L$(TOPDIR)/obj \
	$(NULL)

SOURCES = \
	fte_test.cc \
	fte_hal_mock.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc

GEN_TEST_RESULTS_DIR = $(TOPDIR)/gen/test_results
gtest:
	LD_LIBRARY_PATH=$(TOPDIR)/obj HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/$(PROGRAM) --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/$(PROGRAM).xml"
