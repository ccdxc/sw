
TOPDIR = ../..
SHAREDLIBRARY = libhalpi.so

LCXXFLAGS = \
			-I. \
			-I../svc \
			-I../../gen/protobuf \
			-I../../include \
			-I../../third-party/spdlog/include

SOURCES = \
		  utils.cc \
		  tenant.cc \
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
		  $(NULL)

include $(TOPDIR)/make/Makefile.inc
