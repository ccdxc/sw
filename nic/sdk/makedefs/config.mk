# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
TOP_MAKEFILE  := ${MAKEFILE_LIST}
ifeq "${ARCH}" ""
ARCH := x86_64
endif

BLD_ARCH_DIR := $(abspath ${BLDTOP}/build/${ARCH}/${PIPELINE})
BLD_OUT_DIR  := ${BLD_ARCH_DIR}/out
BLD_LIB_DIR  := ${BLD_ARCH_DIR}/lib
BLD_BIN_DIR  := ${BLD_ARCH_DIR}/bin
BLD_GEN_DIR  := ${BLD_ARCH_DIR}/gen
BLD_OBJ_DIR  := ${BLD_ARCH_DIR}/obj
BLD_DIRS     := ${BLD_OUT_DIR} ${BLD_LIB_DIR} ${BLD_BIN_DIR}

include ${MAKEDEFS}/config_${ARCH}.mk

CMD_NCC      := ${COMMON_CMD_NCC}

