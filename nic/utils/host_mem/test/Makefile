
TOPDIR = ../../..
PROGRAM = host_mem_test
GTEST_ROOT = ../../../third-party/googletest-release-1.8.0/googletest
SYSTEM_LIBRARIES = $(GTEST_ROOT)/make/gtest.a -lpthread -lhostmem -lrt

LCXXFLAGS = \
			-I.. \
			-I../../../include \
			-I../../../third-party/spdlog/include \
			-I$(GTEST_ROOT)/include \
			-L../../../obj \
			-Wno-sign-compare \
			-Wno-uninitialized \
			-Wno-write-strings \
			$(NULL)

SOURCES = \
	host_mem_test.cc \
	$(NULL)


include $(TOPDIR)/make/Makefile.inc

GEN_TEST_RESULTS_DIR = $(TOPDIR)/gen/test_results
gtest:
	LD_LIBRARY_PATH=$(TOPDIR)/obj HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/host_mem_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/host_mem_test.xml"
