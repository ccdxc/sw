TOP = ../../../../..
GTEST = $(TOP)/nic/sdk/third-party/googletest-release-1.8.0/googletest
LDLIBS = $(GTEST)/make/gtest_main.a $(GTEST)/make/gtest.a -lpthread
LDFLAGS = -no-pie
CFLAGS = -g -I.
CXXFLAGS = -g -fpermissive -I. -I$(GTEST)/include

# Since gcc 4.8, fpermissive warns, and the warnings can't be disabled.
#
# Warnings about, eg, "invalid cast from void* to foo*" are expected.  These
# are from c header files with valid c code, and conforming to Linux style
# guidelines, being parsed by the c++ compiler.

#CC = gcc
#CXX = g++

LINK_RECIPE = $(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

all: test_fwh test_res

test: all
	./test_fwh
	./test_res

test_fwh: test_fwh.o
	$(LINK_RECIPE)

test_res: test_res.o wrap_res.o
	$(LINK_RECIPE)

clean:
	rm -f *.o test_fwh test_res

*.o: makefile *.h linux/*.h

.PHONY: all clean test
