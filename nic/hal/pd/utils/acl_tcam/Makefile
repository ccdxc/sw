
TOPDIR = ../../../..
BUILDOBJS = xx
CUSTOM_CLEAN = stubclean

LCXXFLAGS = \
			-I. \
			-I.. \
			-I../tcam \
			-I../../../../include \
			-I../../../../hal/pd \
			-I../../../../gen/iris/include \
			-I../../../../third-party/spdlog/include

SOURCES = \
	acl_tcam.cc \
	$(NULL)

LPOSTALL = \
		   $(TOPDIR)/obj/stub_acl_tcam.o \
		   $(NULL)
LDEPENDS = $(LPOSTALL:.o=.d)

include $(TOPDIR)/make/Makefile.inc

$(LPOSTALL):
	@#echo $(patsubst stub_%, %.cc, $(notdir $(basename $@)))
	$(CXX) -g -include p4pd_stub.hpp -std=c++11 -Wall -Werror -Wuninitialized -fpic -MT $@ -MMD -MP -MF $*.d $(LCXXFLAGS) $(GCOV) -c -o $@ $(patsubst stub_%, %.cc, $(notdir $(basename $@)))

stubclean:
	rm -rf $(LPOSTALL)

