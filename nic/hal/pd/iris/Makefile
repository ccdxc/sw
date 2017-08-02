
TOPDIR = ../../..
SHAREDLIBRARY = libiris.so
SYSTEM_LIBRARIES = -lp4pdapi -lp4pd -lcapri

LCXXFLAGS = \
			-fpic \
			-I. \
			-I.. \
			-I../.. \
			-I../../src \
			-I../common \
			-I/usr/local/include \
			-I../../../include \
			-I../../../gen \
			-I../../../gen/protobuf \
			-I../../../utils/bitmap \
			-I../../../gen/iris/include \
			-I../../../p4/nw/include \
			-I../capri \
			-I../../../third-party/spdlog/include \
			-g \
			$(NULL)

LCFLAGS = $(LCXXFLAGS)


SOURCES = \
		  hal_mem_pd.cc \
		  tenant_pd.cc \
		  nwsec_pd.cc \
		  l2seg_pd.cc \
		  session_pd.cc \
		  lif_pd.cc \
		  uplinkif_pd.cc \
		  enicif_pd.cc \
		  l2seg_uplink_pd.cc \
		  if_pd.cc \
		  endpoint_pd.cc \
		  if_pd_utils.cc \
		  $(NULL)

include $(TOPDIR)/make/Makefile.inc
