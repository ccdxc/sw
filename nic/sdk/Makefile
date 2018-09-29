# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

export ARCH     ?= x86_64
export TOPDIR   := ${realpath ${CURDIR}}
export SDKDIR   := ${TOPDIR}
export BLDTOP   := ${TOPDIR}
export MKDEFS   := ${TOPDIR}/mkinfra
export MKINFRA  := ${TOPDIR}/mkinfra

include ${MKINFRA}/config.mk

.DEFAULT_GOAL := all

.PHONY: all clean
all: ${ALL_TARGETS} 

clean:
	@rm -rf ${BLD_ARCH_DIR}
