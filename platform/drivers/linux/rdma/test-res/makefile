TOP = ../../../../..
GTEST = $(TOP)/nic/sdk/third-party/googletest-release-1.8.0/googletest
LDLIBS = $(GTEST)/make/gtest_main.a $(GTEST)/make/gtest.a -lpthread
LDFLAGS = -no-pie
CFLAGS = -I. -g
CXXFLAGS = -I. -g -I$(GTEST)/include

#CC = gcc
#CXX = g++

test_res: test_res.o wrap_res.o
	$(LINK.cc) $^ $(LOADLIBES) $(LDLIBS) -o $@

clean:
	rm -f *.o test_res

*.o: makefile linux/*.h kpi/linux/*.h
