
TOPDIR = ../../..
PROGRAM = bitmap_test
GTEST_ROOT = ../../../third-party/googletest-release-1.8.0/googletest
SYSTEM_LIBRARIES = -lpthread -lbitmap -ltrace $(GTEST_ROOT)/make/gtest.a

LCXXFLAGS = \
			-I../../../include \
			-I../../../third-party/spdlog/include \
			-I$(GTEST_ROOT)/include \
			-L../../../obj \
			$(NULL)

SOURCES = \
	bitmap_test.cc \
	$(NULL)


include $(TOPDIR)/make/Makefile.inc

GEN_TEST_RESULTS_DIR = $(TOPDIR)/gen/test_results
gtest:
	LD_LIBRARY_PATH=$(TOPDIR)/obj HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/bitmap_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/bitmap_test.xml"
