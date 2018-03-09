
# Copyright (c) 2017, Pensando Systems Inc.

CXX = g++
TOPDIR = ../../../capsim
GENLIB = ../../../capsim/gen/lib
GTEST_ROOT = ../../../third-party/googletest-release-1.8.0/googletest
OBJ_DIR = obj

CAPISA = $(TOPDIR)/lib/capisa

LIBISA = $(TOPDIR)/lib/libisa
LIBMPUOBJ = $(TOPDIR)/lib/libmpuobj
LIBCAPDASM = $(TOPDIR)/lib/libcapdasm
LIBCAPSIM = $(TOPDIR)/lib/libcapsim
LIBASMSYM = $(TOPDIR)/lib/libasmsym

PROGRAM = capsim

LIBRARIES = libasmsym.a libmpuobj.a libcapsim.a libcapdasm.a libisa.a
SYSTEM_LIBRARIES = -lgmp

GENLIBRARIES = $(LIBRARIES:%=$(GENLIB)/%)

LCFLAGS = \
	-I$(LIBISA)/include \
	-I$(LIBCAPDASM)/include \
	-I$(LIBCAPSIM)/include \
	-I$(LIBMPUOBJ)/include \
	-I$(CAPISA)/include \
	-I$(LIBASMSYM)/include \
	-I../../../include/ \
	-I../../../third-party/googletest-release-1.8.0/googletest/include \
	-I../../../gen/nic/asm_out \
	-I$(TOPDIR)/app/capsim/src \
	$(NULL)

LCXXFLAGS = $(LCFLAGS)

LASFLAGS = -I$(CAPISA)

SOURCES = \
	cap_test.cc

CPP_OBJ_FILES := $(patsubst %.cc, %.o, $(SOURCES))
$(CPP_OBJ_FILES): %.o: %.cc
	@echo Compiling $@
	$(CXX) $(LCXXFLAGS) -fpic -std=c++11 -c $< -o $(OBJ_DIR)/$(shell basename $@)

all: $(CPP_OBJ_FILES)
	@echo GENLIBRARIES = $(GENLIBRARIES)
	$(CXX) $(LCXXFLAGS) -Wno-sign-compare sample_asm_test.cc -o $(OBJ_DIR)/sample_asm_test -std=c++11 $(OBJ_DIR)/cap_test.o $(TOPDIR)/gen/app/capsim/src/progmem.o $(TOPDIR)/gen/app/capsim/src/opfile.o $(GTEST_ROOT)/make/gtest.a $(GENLIB)/libcapsim.a $(GENLIB)/libasmsym.a $(GENLIB)/libmpuobj.a $(GENLIB)/libcapdasm.a $(GENLIB)/libisa.a -lpthread $(SYSTEM_LIBRARIES)

test:


