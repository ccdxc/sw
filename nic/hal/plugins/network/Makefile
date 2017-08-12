TOPDIR = ../../..
SHAREDLIBRARY = libnetwork.so

LCXXFLAGS = \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)/third-party/spdlog/include

SOURCES = \
	net_plugin.cc \
	net_fwding.cc \
	net_dfw.cc \
	net_lb.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc
