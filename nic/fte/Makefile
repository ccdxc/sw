TOPDIR = ..
SHAREDLIBRARY = libfte.so

LCXXFLAGS = -I. \
	-I$(TOPDIR)/include \
	-I$(TOPDIR)/hal/src \
	-I$(TOPDIR)/hal/svc \
	-I$(TOPDIR)/gen/protobuf \
	-I$(TOPDIR)/gen/iris/include \
	-I$(TOPDIR)/p4/iris/include \
	-I$(TOPDIR)/asm/cpu-p4+/include \
	-I$(TOPDIR)/third-party/spdlog/include

SOURCES = \
	fte.cc \
	fte_ctx.cc \
	fte_flow.cc \
	fte_session.cc \
	fte_txrx.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc
