# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = naples_status.proto
MODULE_PIPELINE     = iris
MODULE_GEN_TYPES    = CC PY DELPHI GOMETRICS
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/nic \
                      ${TOPDIR}/nic/hal/third-party/google/include \
                      ${TOPDIR}/nic/delphi/proto/delphi \
                      ${TOPDIR}/api/protos \
                      ${TOPDIR}/vendor \
                      ${TOPDIR}/vendor/github.com/pensando/grpc-gateway/third_party/googleapis \
                      /usr/local/include
MODULE_LDLIBS       = pthread
MODULE_POSTGEN_MK   = module_protolib.mk
MODULE_PREREQS = protoc-gen-delphi.gobin
include ${MKDEFS}/post.mk
