
TOPDIR = ../../../..
BUILDOBJS = xx
CUSTOM_CLEAN = stubclean

LCXXFLAGS = \
			-I.. \
			-I../../../../include \
			-I../../../../hal/pd \
			-I../../../../gen/iris/include \
			-I../../../../third-party/spdlog/include

SOURCES = \
	tcam.cc \
	tcam_entry.cc \
	$(NULL)

LPOSTALL = \
		   $(TOPDIR)/obj/stub_tcam.o \
		   $(TOPDIR)/obj/stub_tcam_entry.o \
		   $(NULL)
LDEPENDS = $(LPOSTALL:.o=.d)

include $(TOPDIR)/make/Makefile.inc

$(LPOSTALL):
	@#echo $(patsubst stub_%, %.cc, $(notdir $(basename $@)))
	$(CXX) -include p4pd_stub.hpp -std=c++11 -Wall -Werror -Wuninitialized -fpic -MT $@ -MMD -MP -MF $*.d $(LCXXFLAGS) -c -o $@ $(patsubst stub_%, %.cc, $(notdir $(basename $@)))

stubclean:
	rm -rf $(LPOSTALL)

