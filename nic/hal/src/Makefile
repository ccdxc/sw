
TOPDIR = ../..
SHAREDLIBRARY = libhalpi.so
SYSTEM_LIBRARIES = -lhostmem
SHARED_OBJS :=  $(OBJ_DIR)/utils.o \


LCXXFLAGS = \
			-I. \
			-I.. \
			-I../svc \
			-I../../utils/host_mem \
			-I../../gen/protobuf \
			-I../../gen/iris/include \
                        -I../../gen/common_rxdma_actions/include \
			-I../../include \
			-I../pd/iris \
			-I../pd/capri \
			-I../../asic/capri/model/capsim-master/lib/libcapisa/include \
			-I../../asic/capri/model/capsim-master/lib/libmpuobj/include \
			-I../../third-party/spdlog/include \


SOURCES = \
		  acl.cc \
		  utils.cc \
		  if_utils.cc \
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
		  acl_api.cc \
		  bm_allocator.cc \
		  lif_manager_base.cc \
		  lif_manager.cc \
		  rdma.cc \
		  wring.cc \
		  proxy.cc \
		  descriptor_aol.cc \
		  telemetry.cc \
		  crypto_keys.cc \
		  $(NULL)


include $(TOPDIR)/make/Makefile.inc
