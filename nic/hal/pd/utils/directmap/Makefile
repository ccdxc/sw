
TOPDIR = ../../../..
# SHAREDLIBRARY = libpddirectmap.so
BUILDOBJS = xx
CUSTOM_CLEAN = stubclean

LCXXFLAGS = \
			-I.. \
			-I../../../../include \
			-I../../../../hal/pd \
			-I../../../../gen/iris/include \
			-I../../../../third-party/spdlog/include

SOURCES = \
	directmap.cc \
	$(NULL)

LPOSTALL = ../../../../obj/stub_directmap.o
LDEPENDS = $(TOPDIR)/obj/stub_directmap.d

include $(TOPDIR)/make/Makefile.inc

$(TOPDIR)/obj/stub_directmap.o: 
	$(CXX) -include p4pd_stub.hpp -std=c++11 -Wall -Werror -Wuninitialized -fpic -MT $@ -MMD -MP -MF $*.d $(LCXXFLAGS) -g -c -o $@ directmap.cc
	@# Working
	@#$(CXX) -include p4pd_stub.hpp -std=c++11 -Wall -Werror -Wuninitialized -fpic -MT $(TOPDIR)/obj/stub_directmap.o -MMD -MP -MF $(TOPDIR)/obj/stub_directmap.d $(LCXXFLAGS) -c -o $(TOPDIR)/obj/stub_directmap.o directmap.cc
	@# Not-working
	@# $(CXX) -include p4pd_stub.hpp -std=c++11 -Wall -Werror -Wuninitialized -fpic $(LCXXFLAGS) -MM $< > $(TOPDIR)/obj/stub_directmap.d

stubclean:
	rm -rf $(TOPDIR)/obj/stub_directmap.o

# -include $(TOPDIR)/obj/stub_directmap.d

# GTEST_ROOT = ../../../../third-party/googletest-release-1.8.0/googletest
# GTEST_INC = -I$(GTEST_ROOT)/include
# LDFLAGS = -L$(GENLIB) -lmodelclient -lzmq -lthread -lcapri -ltrace -lpdutils_stub -lpddirectmap
# directmap_test: directmap_test.cc
# 	$(CXX) $(GTEST_INC) $(CXXFLAGS) -Wno-sign-compare directmap_test.cc $(GTEST_ROOT)/make/gtest.a $(LDFLAGS) -o $(GENBIN)/directmap_test
