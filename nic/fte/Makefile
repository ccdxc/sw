TOPDIR = ..
SHAREDLIBRARY = libfte.so

LCXXFLAGS = -I. \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)/hal/src \
	-I$(TOPDIR)/hal/svc \
	-I$(TOPDIR)/gen/protobuf \
	-I$(TOPDIR)/gen/iris/include \
	-I$(TOPDIR)/p4/nw/include \
	-I$(TOPDIR)/third-party/spdlog/include

SOURCES = \
	fte.cc \
	fte_ctx.cc \
	fte_flow.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc
