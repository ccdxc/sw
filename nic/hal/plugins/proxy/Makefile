TOPDIR = ../../..
SHAREDLIBRARY = libproxyplugin.so

LCXXFLAGS = \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)/hal/src \
	-I$(TOPDIR)/hal/svc \
	-I$(TOPDIR)/gen/protobuf \
	-I$(TOPDIR)/build/iris/gen/datapath/p4/include \
	-I$(TOPDIR)/p4/iris/include

SOURCES = \
	proxy_plugin.cc \
	proxy_tcp.cc \
	proxy_tls.cc \
	ipsec.cc \
	app_redir.cc \
	app_redir_cb_ops.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc
