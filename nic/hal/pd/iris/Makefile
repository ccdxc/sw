
TOPDIR = ../../..
SHAREDLIBRARY = libiris.so
SYSTEM_LIBRARIES = -Wl,-rpath $(TOPDIR)/asic/capri/model/capsim-gen/lib -lp4pdapi -lp4pd -lcapri -lp4pdtcprx -lp4pdtcptx -lp4pluspd_rxdma -lp4pluspd_txdma

LCXXFLAGS = \
			-fpic \
			-I. \
			-I.. \
			-I../.. \
			-I../../src \
			-I../../svc \
			-I../common \
			-I/usr/local/include \
			-I../../../include \
			-I../../../gen \
			-I../../../gen/protobuf \
			-I../../../utils/bitmap \
			-I../../../gen/iris/include \
			-I../../../gen/tcp_proxy_rxdma/include \
			-I../../../gen/tcp_proxy_txdma/include \
            -I../../../gen/common_rxdma_actions/include \
            -I../../../gen/common_txdma_actions/include \
			-I../../../gen/tls_txdma_pre_crypto/include \
			-I../../../gen/ipsec_rxdma_actions/include \
			-I../../../gen/cpu_rxdma/include \
            -I../../../gen/common_rxdma_actions/include \
			-I../../../gen/tls_txdma/include \
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
		  rdma_pd.cc \
		  session_pd.cc \
		  lif_pd.cc \
		  uplinkif_pd.cc \
		  uplinkpc_pd.cc \
		  enicif_pd.cc \
		  cpuif_pd.cc \
		  tunnelif_pd.cc \
		  l2seg_uplink_pd.cc \
		  if_pd.cc \
		  endpoint_pd.cc \
		  buf_pool_pd.cc \
		  queue_pd.cc \
		  policer_pd.cc \
		  acl_pd.cc \
		  if_pd_utils.cc \
		  tlscb_pd.cc \
		  tcpcb_pd.cc \
		  wring_pd.cc \
		  descriptor_aol_pd.cc \
		  telemetry_pd.cc \
		  crypto_keys_pd.cc \
		  ipseccb_pd.cc \
		  l4lb_pd.cc \
		  rw_pd.cc \
		  cpucb_pd.cc \
		  cpupkt_api.cc \
		  $(NULL)

include $(TOPDIR)/make/Makefile.inc
