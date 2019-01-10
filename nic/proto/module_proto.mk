# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = device.proto
MODULE_PIPELINE     = iris gft
MODULE_PREREQS      = gogo.proto
MODULE_GEN_TYPES    = CC PY DELPHI
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/nic \
                      ${TOPDIR}/vendor/github.com/gogo/protobuf/gogoproto \
                      ${TOPDIR}/nic/hal/third-party/google/include \
                      ${TOPDIR}/nic/delphi/proto/delphi \
                      /usr/local/include
MODULE_LDLIBS       = pthread
MODULE_POSTGEN_MK   = module.mk 
MODULE_GEN_DIR      = ${BLD_PROTOGEN_DIR}
include ${MKDEFS}/post.mk
