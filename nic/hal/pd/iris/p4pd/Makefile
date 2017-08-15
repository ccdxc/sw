
TOPDIR = ../../../..
SHAREDLIBRARY = libp4pdapi.so

LCXXFLAGS = \
			-fpic \
			-I.. \
			-I../.. \
			-I../../.. \
			-I../../../src \
			-I../../common \
			-I../../../../include \
			-I../../../../gen \
			-I../../../../gen/protobuf \
			-I../../../../utils/bitmap \
			-I../../../../gen/iris/include \
			-I../../../../p4/nw/include \
			-I../../../../p4/include \
			-I../../../../p4/rdma/include \
			-I../../capri \
			-I../../../../third-party/spdlog/include \
			-I/usr/local/include \
			-g \
			$(NULL)

LCFLAGS = $(LCXXFLAGS)

LPOSTALL = p4pdmake

SOURCES = \
		  p4pd_init.cc \
		  p4pd_api.cc \
		  $(NULL)

include $(TOPDIR)/make/Makefile.inc

LCLEANFILES = $(TOPDIR)/obj/p4pd.*

p4pdmake:
	cp Makefile.p4pd ../../../../gen/iris/src/Makefile	
	cp Makefile.tcp_rxdma ../../../../gen/tcp_proxy_rxdma/src/Makefile
	cp Makefile.tcp_txdma ../../../../gen/tcp_proxy_txdma/src/Makefile
