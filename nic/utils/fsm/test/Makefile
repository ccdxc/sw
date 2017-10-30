TOPDIR = ../../..
PROGRAM = fsm_test
GTEST_ROOT = ../../../third-party/googletest-release-1.8.0/googletest
SYSTEM_LIBRARIES = -lpthread -lfsm -ltrace  $(GTEST_ROOT)/make/gtest.a -ltwheel -lslab -lperiodic -lslab -ltwheel -ltrace -lhalmock -lfsm

LCXXFLAGS = \
                        -I../../../include \
                        -I../../../third-party/spdlog/include \
                        -I$(GTEST_ROOT)/include \
                        -L../../../obj \
                        -Wno-sign-compare \
                        $(NULL)

SOURCES = \
        fsm_test.cc \
        $(NULL)


include $(TOPDIR)/make/Makefile.inc

GEN_TEST_RESULTS_DIR = $(TOPDIR)/gen/test_results
gtest:
	make
	LD_LIBRARY_PATH=$(TOPDIR)/obj HAL_CONFIG_PATH=$(TOPDIR)/conf $(TOPDIR)/obj/fsm_test --gtest_output="xml:$(GEN_TEST_RESULTS_DIR)/fsm_test.xml"
