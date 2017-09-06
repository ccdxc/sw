TOPDIR = ../../..
SHAREDLIBRARY = libcapri.so

CAPTOP = $(TOPDIR)/asic/capri/model
CAPMODS = $(wildcard $(CAPTOP)/cap_*)
CAPMODS += $(CAPTOP)/utils
CAPMODS += $(TOPDIR)/asic/ip/verif/pcpp
CAPINC = $(patsubst %, -I%, $(CAPMODS))

LCXXFLAGS = \
			-I. \
			-I./csr \
			-I../.. \
			-I../../src \
			-I../../../include \
			-I../../../third-party/spdlog/include \
			-I/usr/local/include \
			-I../../../asic/capri/model/capsim-master/lib/libmpuobj/include \
			-I../../../asic/capri/model/capsim-master/lib/libcapisa/include \
			-I../../../model_sim/include \
			-I../../../gen/iris/include \
                        -I../../../gen/common_rxdma_actions/include \
                        -I../../../gen/common_txdma_actions/include \
			-I../../../p4/nw/include \
			-I../../../hal/pd \
			-I../../../hal/src \
			-I../../../gen/protobuf \
			$(CAPINC) \
			-g \
			-Wno-maybe-uninitialized \
			$(NULL)

SLFLAGS = \
		  -Wl,--whole-archive $(TOPDIR)/asic/capri/model/capsim-gen/lib/libmpuobj.a $(TOPDIR)/asic/capri/model/capsim-gen/lib/libcapisa.a $(TOPDIR)/asic/capri/model/capsim-gen/lib/libisa.a \
		  -Wl,--no-whole-archive -L$(TOPDIR)/model_sim/build -L/usr/local/lib -L$(TOPDIR)/obj -lmodelclient -lzmq -lcapricsr
SOURCES = \
		  capri_init.cc \
		  capri_asic_rw.cc \
		  capri_config.cc \
		  capri_loader.cc \
		  capri_tbl_rw.cc \
		  capri_hbm.cc \
		  capri_tm_rw.cc \
		  capri_qstate.cc \
		  capri_barco_crypto.cc \
		  $(NULL)

include $(TOPDIR)/make/Makefile.inc
