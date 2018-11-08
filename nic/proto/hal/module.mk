# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = hal.proto
#MODULE_PIPELINE     = iris gft
MODULE_PREREQS      = gogo.proto
MODULE_GEN_TYPES    = CC PY DELPHI
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/nic \
                      ${TOPDIR}/vendor/github.com/gogo/protobuf/gogoproto \
                      ${TOPDIR}/nic/hal/third-party/google/include \
                      ${TOPDIR}/nic/delphi/proto/delphi \
                      /usr/local/include
MODULE_LDLIBS       = pthread
MODULE_POSTGEN_MK   = module_protolib.mk \
                      module_svcgen.mk
MODULE_PREREQS = protoc-gen-delphi.gobin

include ${MKDEFS}/post.mk
