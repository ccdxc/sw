# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = flowstats.proto
MODULE_PIPELINE     = iris gft
MODULE_GEN_TYPES    = CC PY DELPHI GOMETRICS PENMETRICS
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/nic \
                      ${TOPDIR}/nic/hal/third-party/google/include \
                      ${TOPDIR}/nic/delphi/proto/delphi \
                      ${TOPDIR}/venice/utils/apigen/annotations \
                      /usr/local/include
MODULE_LDLIBS       = pthread
MODULE_POSTGEN_MK   = module_protolib.mk
MODULE_PREREQS = protoc-gen-delphi.gobin 
MODULE_GEN_DIR = ${BLD_PROTOGEN_DIR}/flowstats
include ${MKDEFS}/post.mk
