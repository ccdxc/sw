# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
TOP_MAKEFILE  := ${MAKEFILE_LIST}

export BLD_ARCH_DIR        := $(abspath ${BLDTOP}/build/${ARCH}/${PIPELINE})
export BLD_OUT_DIR         := ${BLD_ARCH_DIR}/out
export BLD_LIB_DIR         := ${BLD_ARCH_DIR}/lib
export BLD_BIN_DIR         := ${BLD_ARCH_DIR}/bin
export BLD_GEN_DIR         := ${BLD_ARCH_DIR}/gen
export BLD_SVCGEN_DIR      := ${BLD_GEN_DIR}/hal/svc
export BLD_P4GEN_DIR       := ${BLD_GEN_DIR}/p4gen
export BLD_P4CLICMN_DIR    := ${BLD_GEN_DIR}/p4genclicommon
export BLD_PROTOGEN_DIR    := ${BLD_GEN_DIR}/proto
export BLD_PGMBIN_DIR      := ${BLD_ARCH_DIR}/pgm_bin
export BLD_ASMBIN_DIR      := ${BLD_BIN_DIR}/asm
export BLD_SYM_DIR         := ${BLD_ARCH_DIR}/sym

export TMAGENT_PEN_IN_DIR  := ${GOPATH}/src/github.com/pensando/sw/nic/agent/protos/templates/restapi/tmagent
export TMAGENT_PEN_OUT_DIR := ${TOPDIR}/nic/agent/protos/generated/restapi/tmagent
export PEN_IN_DIR          := ${GOPATH}/src/github.com/pensando/sw/penctl/cmd/metrics/templates
export PEN_OUT_DIR         := ${TOPDIR}/penctl/cmd
export PDSA_SVC_IN_DIR     := ${GOPATH}/src/github.com/pensando/sw/nic/metaswitch/stubs/templates/svc
export PDSA_SVC_OUT_DIR    := ${TOPDIR}/nic/metaswitch/stubs/mgmt/gen/svc
export PDSA_MGMT_IN_DIR    := ${GOPATH}/src/github.com/pensando/sw/nic/metaswitch/stubs/templates/mgmt
export PDSA_MGMT_OUT_DIR   := ${TOPDIR}/nic/metaswitch/stubs/mgmt/gen/mgmt

include ${MKINFRA}/config_${ARCH}.mk

CMD_NCC      := ${COMMON_CMD_NCC}
CMD_SORRENTO := ${COMMON_CMD_SORRENTO}
CMD_P4C_OPTS := ${COMMON_P4C_OPTS}

CMD_P4C_ASM  := ${TOPDIR}/nic/tools/sorrento/tools/p4c_asm.sh

CMD_CAPAS       := ${COMMON_CMD_CAPAS}
CMD_CAPAS_OPTS  := ${COMMON_CAPAS_OPTS}

CMD_PROTOC_LDLIB_PATH   := ${TOPDIR}/nic/hal/third-party/google/x86_64/lib/
CMD_PROTOC_PLUGIN_PATH   := ${TOPDIR}/nic/build/x86_64/iris/bin/
CMD_PROTOC              := LC_ALL=C PATH=${CMD_PROTOC_PLUGIN_PATH}:$$(PATH) LD_LIBRARY_PATH=${CMD_PROTOC_LDLIB_PATH} ${TOPDIR}/nic/hal/third-party/google/x86_64/bin/protoc
CMD_PROTOC_C            := ${TOPDIR}/nic/hal/third-party/google/x86_64/bin/protoc-c
CMD_MOCKGEN             := mockgen
CMD_GOIMPORTS           := goimports -local "github.com/pensando/sw"
CMD_SWIG                := swig

CMD_PROTOC_CPP_OPTS     := --plugin=protoc-gen-grpc=${GRPC_CPP_PLUGIN}
CMD_PROTOC_PY_OPTS      := --plugin=protoc-gen-grpc=${GRPC_PY_PLUGIN}

CMD_LINKER_FLAGS        := -pthread -rdynamic -no-canonical-prefixes \
                           -Wl,--gc-sections -Wl,-z,relro,-z,now \
                           -Wl,--build-id=md5 -Wl,--hash-style=gnu \
                           ${ARCH_LINKER_FLAGS}

ifneq "$(dir ${MAKECMDGOALS})" "docker/"
include ${MKINFRA}/generator.mk
endif
