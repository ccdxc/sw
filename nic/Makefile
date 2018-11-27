# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
.SUFFIXES:
export ARCH         ?= x86_64
export PIPELINE     ?= iris
export TOPDIR       := ${realpath ${CURDIR}/../}
export NICDIR       := ${TOPDIR}/nic
export IOTADIR      := ${TOPDIR}/iota
export SDKDIR       := ${NICDIR}/sdk
export BLDTOP       := ${realpath .}
export MKINFRA      := ${SDKDIR}/mkinfra
export MKDEFS       := ${NICDIR}/mkdefs
export COVDIR       := ${NICDIR}/coverage
export COVFILE_NAME := bullseye_hal.cov
export COVFILE      := ${COVDIR}/${COVFILE_NAME}

ifeq ($(COVERAGE),1)
    export COVERAGE_OPTS := --coveragerun
    export SUDO_OPT := sudo
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
