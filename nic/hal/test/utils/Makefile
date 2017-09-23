
TOPDIR = ../../..
SHAREDLIBRARY = libhaltestutils.so

LCXXFLAGS = \
			-I. \
			-I$(TOPDIR)/hal/pd/iris \
			-I$(TOPDIR)/gen/iris/include \
			-I$(TOPDIR)/gen/common_rxdma_actions/include \
			-I$(TOPDIR)/gen/common_txdma_actions/include \
			-I$(TOPDIR)/include \
			-I$(TOPDIR)/third-party/spdlog/include

SOURCES = \
	hal_test_utils.cc \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc
