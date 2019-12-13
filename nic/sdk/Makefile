# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

export ARCH     ?= x86_64
export FWTYPE   ?= full
export TOPDIR   := ${realpath ${CURDIR}}
export SDKDIR   := ${TOPDIR}
export BLDTOP   := ${TOPDIR}
export MKINFRA  := ${TOPDIR}/mkinfra
export MKDEFS   := ${TOPDIR}/mkdefs
export TOOLSDIR := ${TOPDIR}/tools/
export PIPELINE :=

include ${MKDEFS}/common.mk
include ${MKINFRA}/config.mk

.DEFAULT_GOAL := all

.PHONY: all clean
all: ${ALL_TARGETS}

clean:
	@rm -rf ${BLD_ARCH_DIR}
	@rm -rf ${CLEAN_DIRS}
