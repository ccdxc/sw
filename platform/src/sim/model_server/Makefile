#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = model_server
SOURCES = model_server.cc sim_server.c zmq_wait.c lib_driver.cc
LBUILD_ARCHES = x86_64

TOPDIR = ../../..
NIC = $(TOPDIR)/../nic

ARCHIVES =  -Wl,--allow-multiple-definition
ARCHIVES += $(NIC)/hal/third-party/grpc/$(LBUILD_ARCHES)/lib/libgrpc*.a -Wl,--no-whole-archive

# cancel -Wall -Werror
CCWARN_FLAGS = $(NULL)
LCFLAGS = \
	-I /usr/include/python2.7 \
	-I $(NIC)/model_sim/include \
	-I ../../lib/misc/include \
	-I ../../lib/pciehsvc/include \
	-I ../libsimlib/include \
	-I ../libsimdev/include \
	-I ../../../../ \
	-I $(NIC)/hal/third-party/google/include \
	-I $(NIC)/hal/third-party/grpc/include \
	$(NULL)
LCXXFLAGS = $(LCFLAGS)
LLDFLAGS = \
	-L $(NIC)/model_sim/libs \
	$(NULL)

LIBRARIES = \
	-lsimlib \
	-lsimdev \
	-lpciehsvc \
	-lpcieport \
	-lpciehw \
	-lpcietlp \
	-lintrutils \
	-lpciehost \
	-lcfgspace \
	-lintrutils \
	-lpal \
	-lmisc \
	$(NULL)

SYSTEM_LIBRARIES = \
	$(ARCHIVES) \
	-lsknobs \
	-lutil \
	-lgmp \
	-lgmpxx \
	-lpython2.7 \
	-lmodel_cc \
	-lcommon_cc \
	-lmpuobj \
	-lJudy \
	-lsknobs \
	-ltop_csr_compile \
	-lcapsim \
	-lisa \
	-lasmsym \
	-lzmq \
	-lcapisa \
	-lcrypto \
	-lpthread \
	-ldl \
	-L$(NIC)/gen/x86_64/lib -lhalproto \
	$(NIC)/hal/third-party/google/x86_64/lib/libprotobuf.so.14 \
	-lz \
	$(NULL)

include $(TOPDIR)/make/Makefile.inc
