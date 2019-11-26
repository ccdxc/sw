# {C} Copyright 2019 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = pdsa.proto
MODULE_PIPELINE     = apollo apulu artemis
MODULE_GEN_TYPES    = CC
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/nic \
                      ${TOPDIR}/nic/hal/third-party/google/include \
                      ${TOPDIR}/nic/delphi/proto/delphi \
                      ${TOPDIR}/venice/utils/apigen/annotations \
                      /usr/local/include
MODULE_LDLIBS       = pthread
MODULE_POSTGEN_MK   = module_protolib.mk
MODULE_SRCS         = ${TOPDIR}/venice/utils/apigen/annotations/pdsa.proto

include ${MKDEFS}/post.mk
