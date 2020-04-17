# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET       = gogo.proto
MODULE_PIPELINE     = iris gft apollo artemis apulu athena
MODULE_GEN_TYPES    = CC PY
MODULE_INCS         = ${MODULE_DIR} \
                      ${TOPDIR}/nic \
                      ${TOPDIR}/vendor/github.com/gogo/protobuf/gogoproto \
                      ${TOPDIR}/nic/hal/third-party/google/include
MODULE_SRC_DIR      = ${TOPDIR}/vendor/github.com/gogo/protobuf/gogoproto
include ${MKDEFS}/post.mk
