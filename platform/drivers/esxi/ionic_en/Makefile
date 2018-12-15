# Copyright 2014-2016 VMware, Inc.  All rights reserved. -- VMware Confidential

#-------------------------------------------------------------------------------
# Makefile
#
#    Provides a wrapper to compile drivers in the VMKAPI DDK devkit.
#
#    This is a generic Makefile and no changes are required to compile
#    drivers provided that a single .sc file exists in the current directory.
#-------------------------------------------------------------------------------

#-------------------------------------------------------------------------------
# Tools
#-------------------------------------------------------------------------------

CAYMAN_ESX_PYCPARSER_DIR = $(shell /bin/rpm -q --qf "%{instprefixes}\n" vmware-esx-cayman_esx_pycparser-7bc084d155833aecf70a98ec2c63c756e4d97fc5-toolchain)
export GOBUILD_CAYMAN_ESX_PYCPARSER_ROOT = $(CAYMAN_ESX_PYCPARSER_DIR)
CAYMAN_ESX_GLIBC_2_17_DIR = $(shell /bin/rpm -q --qf "%{instprefixes}\n" vmware-esx-cayman_esx_glibc_2_17-780e3316a131e543d4f78b2fecaaf3c21f4527f4-toolchain)
export GOBUILD_CAYMAN_ESX_GLIBC_2_17_ROOT = $(CAYMAN_ESX_GLIBC_2_17_DIR)
CAYMAN_ESX_TOOLCHAIN_DIR = $(shell /bin/rpm -q --qf "%{instprefixes}\n" vmware-esx-cayman_esx_toolchain-34b52da77344ab2f5cd0bd80f7dfd85b911a5ce3-toolchain)
export GOBUILD_CAYMAN_ESX_TOOLCHAIN_ROOT = $(CAYMAN_ESX_TOOLCHAIN_DIR)
VMW_COMPONENTS = cayman_esx_pycparser cayman_esx_glibc_2_17 cayman_esx_toolchain
VMW_COMPONENT_DIRS = $(CAYMAN_ESX_PYCPARSER_DIR) $(CAYMAN_ESX_GLIBC_2_17_DIR) $(CAYMAN_ESX_TOOLCHAIN_DIR)

AWK = /build/toolchain/lin32/gawk-3.1.6/bin/awk
BASH = /build/toolchain/lin32/bash-3.2/bin/bash
CP = /build/toolchain/lin32/coreutils-5.97/bin/cp
ECHO = /build/toolchain/lin32/coreutils-5.97/bin/echo
GREP = /build/toolchain/lin32/grep-2.5.1a/bin/grep
GZIP = /build/toolchain/lin32/gzip-1.3.5/bin/gzip
MAKE = /build/toolchain/lin32/make-3.81/bin/make
MKDIR = /build/toolchain/lin32/coreutils-5.97/bin/mkdir
MV = /build/toolchain/lin32/coreutils-5.97/bin/mv
PERL = /build/toolchain/lin32/perl-5.8.8/bin/perl
RM = /build/toolchain/lin32/coreutils-5.97/bin/rm
SED = /build/toolchain/lin32/sed-4.1.5/bin/sed
TR = /build/toolchain/lin32/coreutils-8.6/bin/tr

#-------------------------------------------------------------------------------
# Parameters to the scons command
#-------------------------------------------------------------------------------
PRODUCT      = nativeddk
BUILDTYPE    = beta
BUILD_NUMBER = 8169922

VMKAPI_DIR   = $(shell /bin/rpm -q --qf '%{INSTPREFIXES}\n' vmware-esx-nativeddk-devtools-6.7.0-0.0.8169922)
SRC          = $(VMKAPI_DIR)/src
FIND_RPM     = $(VMKAPI_DIR)/tools/find_rpm_install_dir.sh
VIRTUAL_GOBUILD = $(VMKAPI_DIR)/tools/gobuild/nativeddk.cache
VIBTOOLS     := $(shell $(FIND_RPM) vmware-esx-vib-suite)
ifeq ($(strip $(VIBTOOLS)),)
    $(error Unable to locate VIB Suite installation directory)
endif

SCONS        = cd $(SRC); $(SRC)/scons/bin/scons
FULL_PWD     := $(shell pwd)
SC           := $(shell echo *.sc | $(AWK) '{print $$1}')
MANIFEST     = $(FULL_PWD)/$(SC)
BUILD        = build
SCONS_BUILD  = .build
TARGET       = $(FULL_PWD)/$(BUILD)
SCONS_TARGET = $(FULL_PWD)/$(SCONS_BUILD)

# Map VIB acceptance level to canonical default signing-key file base name
ACCEPT_LEVEL = $(shell grep acceptance-level $(MANIFEST))
ifneq ($(findstring accepted,$(ACCEPT_LEVEL)),)
    VMW_KC_NAME = accepted
else
ifneq ($(findstring partner,$(ACCEPT_LEVEL)),)
    VMW_KC_NAME = vmpartner
else
ifneq ($(findstring certified,$(ACCEPT_LEVEL)),)
    VMW_KC_NAME = vmware
else
ifeq ($(findstring community,$(ACCEPT_LEVEL)),)
    $(error invalid acceptance level $(ACCEPT_LEVEL))
endif
endif
endif
endif
ifneq ($(VMW_KC_NAME),)
    KEYPATH?=$(VIBTOOLS)/testcerts/$(VMW_KC_NAME).key
    CERTPATH?=$(VIBTOOLS)/testcerts/$(VMW_KC_NAME).cert
endif

all vib:
	@$(SCONS) native-driver-modules native-driver-vibs native-driver-bundle \
	       RELEASE_PACKAGES_DIR=$(TARGET) \
	       TOOLKIT_MANIFEST=$(MANIFEST) \
	       PRODUCT=$(PRODUCT) \
	       BUILDTYPE=$(BUILDTYPE) \
	       BUILD_NUMBER=$(BUILD_NUMBER) \
	       ESX_SIGN_BINARIES=0 \
	       SKIP_BIG_BROTHER=1 \
	       VIBPATH=$(VIBTOOLS) \
	       BUILDROOT=$(SCONS_TARGET) \
	       VIRTUAL_GOBUILD=$(VIRTUAL_GOBUILD) \
               KEYPATH=$(KEYPATH) \
               CERTPATH=$(CERTPATH)

clean:
	@echo Cleaning out build directories ...
	@$(RM) -rf $(TARGET) $(SCONS_TARGET)
	@echo Done.
