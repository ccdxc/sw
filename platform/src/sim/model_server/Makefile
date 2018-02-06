#
# Copyright (c) 2017, Pensando Systems Inc.
#

PROGRAM = model_server
SOURCES = model_server.cc sim_server.c zmq_wait.c
LBUILD_ARCHES = x86_64

NIC = ../../../../nic

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
	$(NULL)
LCXXFLAGS = $(LCFLAGS)
LLDFLAGS = \
	-L $(NIC)/model_sim/libs \
	$(NULL)

LIBRARIES = \
	-lsimlib \
	-lsimdev \
	-lpciehsvc \
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
	$(NULL)

TOPDIR = ../../..
include $(TOPDIR)/make/Makefile.inc
