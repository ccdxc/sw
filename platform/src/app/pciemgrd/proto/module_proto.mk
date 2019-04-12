# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = pciemgr.proto
MODULE_PIPELINE     = iris gft
MODULE_GEN_TYPES    = CC DELPHI GOMETRICS PENMETRICS
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/nic \
                      ${TOPDIR}/nic/hal/third-party/google/include \
                      ${TOPDIR}/nic/delphi/proto/delphi \
                      ${TOPDIR}/venice/utils/apigen/annotations \
                      /usr/local/include
MODULE_LDLIBS       = pthread
MODULE_POSTGEN_MK   = module_protolib.mk
MODULE_PREREQS	    = protoc-gen-delphi.gobin penctl.proto
MODULE_GEN_DIR      = ${BLD_PROTOGEN_DIR}/pciemgr
include ${MKDEFS}/post.mk
