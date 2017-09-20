
TOPDIR = ../..
SHAREDLIBRARY = liblkl_api.so

LIBLKL_ROOT = ../../third-party/lkl/export/
LIBLKL_INC = $(LIBLKL_ROOT)/include

CPPFLAGS += -I$(LIBLKL_INC) -DCONFIG_AUTO_LKL_POSIX_HOST

LCXXFLAGS = \
			-I. \
			-I.. \
			-I../svc \
			-I../../utils/host_mem \
			-I../../gen/protobuf \
			-I../../gen/iris/include \
			-I../../gen/common_rxdma_actions/include \
			-I../../gen/common_txdma_actions/include \
			-I../../include \
			-I../../p4/include \
			-I../pd/iris \
			-I../pd/capri \
			-I../../asic/capri/model/capsim-master/lib/libcapisa/include \
			-I../../asic/capri/model/capsim-master/lib/libmpuobj/include \
			-I../../third-party/spdlog/include \
			-I$(LIBLKL_INC) \

SOURCES = \
		  lkl_api.cc \
		  $(NULL)


include $(TOPDIR)/make/Makefile.inc
