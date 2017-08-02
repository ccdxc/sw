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
