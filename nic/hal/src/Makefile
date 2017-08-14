
TOPDIR = ../..
SHAREDLIBRARY = libhalpi.so

LCXXFLAGS = \
			-I. \
			-I.. \
			-I../svc \
			-I../../gen/protobuf \
			-I../../gen/iris/include \
			-I../../include \
			-I../../third-party/spdlog/include

SOURCES = \
		  utils.cc \
		  tenant.cc \
		  network.cc \
		  interface.cc \
		  l2segment.cc \
		  endpoint.cc \
		  session.cc \
		  nwsec.cc \
		  tlscb.cc \
		  tcpcb.cc \
		  qos.cc \
		  interface_api.cc \
		  l2segment_api.cc \
		  endpoint_api.cc \
		  nwsec_api.cc \
		  qos_api.cc \
		  bm_allocator.cc \
		  lif_manager_base.cc \
		  lif_manager.cc \
		  $(NULL)

include $(TOPDIR)/make/Makefile.inc
