TOPDIR = ../../..
SHAREDLIBRARY = libproxyplugin.so

LCXXFLAGS = \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)/hal/src \
	-I$(TOPDIR)/hal/svc \
	-I$(TOPDIR)/gen/protobuf \
	-I$(TOPDIR)/gen/iris/include \
	-I$(TOPDIR)/p4/nw/include \
	-I$(TOPDIR)/third-party/spdlog/include

SOURCES = \
	proxy_plugin.cc \
	proxy_tcp.cc \
	proxy_tls.cc \
	ipsec.cc \
	app_redir.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc
