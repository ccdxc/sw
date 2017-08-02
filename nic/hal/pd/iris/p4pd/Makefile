
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

p4pdmake:
	cp Makefile.p4pd ../../../../gen/iris/src/Makefile
