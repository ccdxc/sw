
TOPDIR = ../../../..
# SHAREDLIBRARY = libpddirectmap.so
BUILDOBJS = xx

LCXXFLAGS = \
			-I.. \
			-I$(TOPDIR)/include \
			-I$(TOPDIR)/hal/pd \
			-I$(TOPDIR)/gen/iris/include \
			-I$(TOPDIR)/third-party/spdlog/include

SOURCES = \
	flow.cc \
	flow_entry.cc \
	flow_hint_group.cc \
	flow_spine_entry.cc \
	flow_table_entry.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc

