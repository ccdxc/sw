
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
	flow.cc \
	flow_entry.cc \
	flow_hint_group.cc \
	flow_spine_entry.cc \
	flow_table_entry.cc \
	$(NULL)

LPOSTALL = \
		   $(TOPDIR)/obj/stub_flow.o \
		   $(TOPDIR)/obj/stub_flow_entry.o \
		   $(TOPDIR)/obj/stub_flow_hint_group.o \
		   $(TOPDIR)/obj/stub_flow_spine_entry.o \
		   $(TOPDIR)/obj/stub_flow_table_entry.o \
		   $(NULL)
LDEPENDS = $(LPOSTALL:.o=.d)

include $(TOPDIR)/make/Makefile.inc

$(LPOSTALL):
	@#echo $(patsubst stub_%, %.cc, $(notdir $(basename $@)))
	$(CXX) -g -include p4pd_stub.hpp -std=c++11 -Wall -Werror -Wuninitialized -fpic -MT $@ -MMD -MP -MF $*.d $(LCXXFLAGS) -c -o $@ $(patsubst stub_%, %.cc, $(notdir $(basename $@)))

stubclean:
	rm -rf $(LPOSTALL)

