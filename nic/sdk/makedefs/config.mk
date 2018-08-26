# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
TOP_MAKEFILE  := ${MAKEFILE_LIST}
ifeq "${ARCH}" ""
ARCH := x86_64
endif

ifeq "${PIPELINE}" ""
PIPELINE := iris
endif

BLD_ARCH_DIR    := $(abspath ${BLDTOP}/build/${ARCH}/${PIPELINE})
BLD_OUT_DIR     := ${BLD_ARCH_DIR}/out
BLD_LIB_DIR     := ${BLD_ARCH_DIR}/lib
BLD_BIN_DIR     := ${BLD_ARCH_DIR}/bin
BLD_GEN_DIR     := ${BLD_ARCH_DIR}/gen
BLD_PGMBIN_DIR  := ${BLD_ARCH_DIR}/pgm_bin
BLD_ASMBIN_DIR  := ${BLD_ARCH_DIR}/asm_bin
BLD_SYM_DIR     := ${BLD_ARCH_DIR}/sym

include ${MAKEDEFS}/config_${ARCH}.mk

CMD_NCC      := ${COMMON_CMD_NCC}
CMD_NCC_OPTS := ${COMMON_NCC_OPTS}

CMD_CAPAS       := ${COMMON_CMD_CAPAS}
CMD_CAPAS_OPTS  := ${COMMON_CAPAS_OPTS}
