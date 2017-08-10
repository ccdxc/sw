
TOPDIR = ../../..
SHAREDLIBRARY = libiris.so
SYSTEM_LIBRARIES = -lp4pdapi -lp4pd -lcapri -lp4pdtcprx -lp4pdtcptx

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
			-I../../../gen/tcp_proxy_rxdma/include \
			-I../../../gen/tcp_proxy_txdma/include \
			-I../../../p4/nw/include \
			-I../capri \
			-I../../../third-party/spdlog/include \
			-I../../../asic/capri/model/capsim-master/lib/libcapisa/include \
			-I../../../asic/capri/model/capsim-master/lib/libmpuobj/include \
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
		  uplinkpc_pd.cc \
		  enicif_pd.cc \
		  l2seg_uplink_pd.cc \
		  if_pd.cc \
		  endpoint_pd.cc \
		  buf_pool_pd.cc \
		  queue_pd.cc \
		  policer_pd.cc \
		  if_pd_utils.cc \
		  tlscb_pd.cc \
		  tcpcb_pd.cc \
		  $(NULL)

include $(TOPDIR)/make/Makefile.inc
