
TOPDIR = ../../..
PROGRAM = twheel_test
GTEST_ROOT = ../../../third-party/googletest-release-1.8.0/googletest
SYSTEM_LIBRARIES = -lpthread -ltwheel -lslab -lperiodic -lslab -ltwheel -ltrace -lhalmock $(GTEST_ROOT)/make/gtest.a

LCXXFLAGS = \
			-I../../../include \
			-I../../../third-party/spdlog/include \
			-I$(GTEST_ROOT)/include \
			-L../../../obj \
			-Wno-sign-compare \
			$(NULL)

SOURCES = \
	twheel_test.cc \
	$(NULL)


include $(TOPDIR)/make/Makefile.inc

GEN_TEST_RESULTS_DIR = $(TOPDIR)/gen/test_results
gtest:
	LD_LIBRARY_PATH=$(TOPDIR)/obj HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/twheel_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/twheel_test.xml"
