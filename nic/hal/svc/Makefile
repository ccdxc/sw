
TOPDIR = ../..
SHAREDLIBRARY = libsvc.so

GEN_DIR = $(TOPDIR)/gen
LCXXFLAGS = \
			-I. \
			-I$(GEN_DIR)/protobuf \
			-I$(GEN_DIR)/iris/include \
			-I../src \
			-I../../include \
			-I../../third-party/spdlog/include



SOURCES = \
		vrf_svc.cc \
		network_svc.cc \
		interface_svc.cc \
		internal_svc.cc \
		l2segment_svc.cc \
		endpoint_svc.cc \
		session_svc.cc \
		nwsec_svc.cc \
		l4lb_svc.cc \
		tlscb_svc.cc \
		tcpcb_svc.cc \
		descriptor_aol_svc.cc \
		qos_svc.cc \
		acl_svc.cc \
		wring_svc.cc \
		proxy_svc.cc \
		telemetry_svc.cc \
		crypto_keys_svc.cc \
		ipseccb_svc.cc \
 		rdma_svc.cc \
		cpucb_svc.cc \
		rawrcb_svc.cc \
		rawccb_svc.cc \
		system_svc.cc \
		$(NULL)

include $(TOPDIR)/make/Makefile.inc
