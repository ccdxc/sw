# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
.SUFFIXES:
export ARCH     ?= x86_64
export PIPELINE ?= iris
export TOPDIR   := ${realpath ${CURDIR}/../}
export NICDIR   := ${TOPDIR}/nic
export SDKDIR   := ${NICDIR}/sdk
export BLDTOP   := ${realpath .}
export MKINFRA  := ${SDKDIR}/mkinfra
export MKDEFS   := ${NICDIR}/mkdefs

include ${MKDEFS}/common.mk
include ${MKINFRA}/config.mk

.DEFAULT_GOAL := package
.PHONY: all clean
all: ${ALL_TARGETS} 

clean: package-clean
	@rm -rf ${BLD_ARCH_DIR}
	@rm -rf ${CLEAN_DIRS}
