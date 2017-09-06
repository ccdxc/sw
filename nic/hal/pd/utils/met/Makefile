
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
	met.cc \
	repl_list.cc \
	repl_table_entry.cc \
	repl_entry.cc \
	$(NULL)

LPOSTALL = \
		   $(TOPDIR)/obj/stub_met.o \
		   $(TOPDIR)/obj/stub_repl_list.o \
		   $(TOPDIR)/obj/stub_repl_table_entry.o \
		   $(TOPDIR)/obj/stub_repl_entry.o \
		   $(NULL)
LDEPENDS = $(LPOSTALL:.o=.d)

include $(TOPDIR)/make/Makefile.inc

$(LPOSTALL):
	@#echo $(patsubst stub_%, %.cc, $(notdir $(basename $@)))
	$(CXX) -include p4pd_stub.hpp -g -std=c++11 -Wall -Werror -Wuninitialized -fpic -MT $@ -MMD -MP -MF $*.d $(LCXXFLAGS) $(GCOV) -c -o $@ $(patsubst stub_%, %.cc, $(notdir $(basename $@)))

stubclean:
	rm -rf $(LPOSTALL)

