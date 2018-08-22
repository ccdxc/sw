# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
TOP_MAKEFILE  := ${MAKEFILE_LIST}
ifeq "${ARCH}" ""
ARCH := x86_64
endif

BLD_ARCH_DIR := build/${ARCH}
BLD_OUT_DIR  := ${BLD_ARCH_DIR}/out
BLD_LIB_DIR  := ${BLD_ARCH_DIR}/lib
BLD_BIN_DIR  := ${BLD_ARCH_DIR}/bin
BLD_DIRS     := ${BLD_OUT_DIR} ${BLD_LIB_DIR} ${BLD_BIN_DIR}

include makedefs/config_${ARCH}.mk

