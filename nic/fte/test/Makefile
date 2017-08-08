TOPDIR = ../..
PROGRAM = fte_test
GTEST_ROOT = $(TOPDIR)/third-party/googletest-release-1.8.0/googletest
SYSTEM_LIBRARIES = -lpthread -lfte -ltrace $(GTEST_ROOT)/make/gtest.a

LCXXFLAGS = \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)//third-party/spdlog/include \
	-I$(GTEST_ROOT)/include \
	-L$(TOPDIR)/obj \
	$(NULL)

SOURCES = \
	fte_test.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc

GEN_TEST_RESULTS_DIR = $(TOPDIR)/gen/test_results
gtest:
	LD_LIBRARY_PATH=$(TOPDIR)/obj HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/$(PROGRAM) --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/$(PROGRAM).xml"
