TOPDIR = ../../..
SHAREDLIBRARY = libnetwork.so

LCXXFLAGS = \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)/hal/src \
	-I$(TOPDIR)/hal/svc \
	-I$(TOPDIR)/gen/protobuf \
	-I$(TOPDIR)/gen/iris/include \
	-I$(TOPDIR)/p4/nw/include \
	-I$(TOPDIR)/p4/include \
	-I$(TOPDIR)/third-party/spdlog/include

SOURCES = \
	net_plugin.cc \
	net_stage.cc \
	net_tunnel.cc \
	net_fwding.cc \
	net_dfw.cc \
	net_lb.cc \
	dol_test.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc
