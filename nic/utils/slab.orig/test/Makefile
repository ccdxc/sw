
TOPDIR = ../../..
PROGRAM = slab_test
GTEST_ROOT = ../../../third-party/googletest-release-1.8.0/googletest
SYSTEM_LIBRARIES = -lpthread -lslab -lperiodic -lslab -ltwheel -lhalmock -ltrace $(GTEST_ROOT)/make/gtest.a

LCXXFLAGS = \
			-I../../../include \
			-I../../../third-party/spdlog/include \
			-I$(GTEST_ROOT)/include \
			-L../../../obj \
			-Wno-sign-compare \
			$(NULL)

SOURCES = \
	slab_test.cc \
	$(NULL)


include $(TOPDIR)/make/Makefile.inc

GEN_TEST_RESULTS_DIR = $(TOPDIR)/gen/test_results
gtest:
	LD_LIBRARY_PATH=$(TOPDIR)/obj HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/slab_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/slab_test.xml"
