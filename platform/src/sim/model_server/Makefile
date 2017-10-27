#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = model_server
SOURCES = model_server.cc sim_server.c zmq_wait.c
LBUILD_ARCHES = x86_64

ASIC_TOOLS = /home/asic/tools/src/0.25
NIC = ../../../pensando/sw/nic

# cancel -Wall -Werror
CCWARN_FLAGS = $(NULL)
LCFLAGS = \
	-I $(ASIC_TOOLS)/include \
	-I $(NIC)/model_sim/include \
	-I /usr/include/python2.7 \
	-I ../../lib/misc/include \
	-I ../../lib/pciehsvc/include \
	-I ../libsimlib/include \
	-I ../libsimdev/include \
	$(NULL)
LCXXFLAGS = $(LCFLAGS)
LLDFLAGS = \
	-L $(ASIC_TOOLS)/x86_64/lib64 \
	-L $(NIC)/model_sim/libs \
	$(NULL)

LIBRARIES = \
	-lsimlib \
	-lsimdev \
	-lpciehsvc \
	-lpciehw \
	-lpciehost \
	-lcfgspace \
	-lmisc \
	$(NULL)

SYSTEM_LIBRARIES = \
	-lsknobs \
	-lutil \
	-lgmp \
	-lgmpxx \
	-lpython2.7 \
	-lmodel_cc \
	-lcommon_cc \
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
	$(NULL)

TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
