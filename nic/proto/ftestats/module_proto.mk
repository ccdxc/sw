# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = ftestats.proto
MODULE_PIPELINE     = iris gft
MODULE_GEN_TYPES    = CC PY DELPHI GOMETRICS
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/nic \
                      ${TOPDIR}/nic/hal/third-party/google/include \
                      ${TOPDIR}/nic/delphi/proto/delphi \
                      /usr/local/include
MODULE_LDLIBS       = pthread
MODULE_POSTGEN_MK   = module_protolib.mk
MODULE_PREREQS = protoc-gen-delphi.gobin
MODULE_GEN_DIR = ${BLD_PROTOGEN_DIR}/ftestats
include ${MKDEFS}/post.mk
