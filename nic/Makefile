# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
.SUFFIXES:
export ARCH         ?= x86_64
export PIPELINE     ?= iris
export PLATFORM     ?= sim
export ASIC         ?= capri
export FWTYPE       ?= full
export P4VER        ?= P4_14
export TOPDIR       := ${realpath ${CURDIR}/../}
export NICDIR       := ${TOPDIR}/nic
export IOTADIR      := ${TOPDIR}/iota
export SDKDIR       := ${NICDIR}/sdk
export BLDTOP       := ${realpath .}
export MKINFRA      := ${SDKDIR}/mkinfra
export MKDEFS       := ${NICDIR}/mkdefs
export COVDIR       := ${NICDIR}/coverage

ifeq ($(COVERAGE),1)
    export COVERAGE_OPTS := --coveragerun
    export SUDO_OPT := sudo
    # Coverage needs to know path of arm
	export PATH := ${PATH}:/tool/toolchain/aarch64-1.1/bin
	ifeq ($(ARCH),aarch64)
	    export COVFILE_NAME := hw_bullseye_hal.cov
	else
	    export COVFILE_NAME := sim_bullseye_hal.cov
	endif
    export COVFILE := ${COVDIR}/${COVFILE_NAME}
.ONESHELL:
endif

include ${MKDEFS}/common.mk
include ${MKINFRA}/config.mk

.DEFAULT_GOAL := package
.PHONY: all clean
all: ${ALL_TARGETS} 

clean: package-clean
	@rm -rf ${BLD_ARCH_DIR}
	@rm -rf ${CLEAN_DIRS}
