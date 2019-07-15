# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libdol_engine.so
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/dole.cc \
                  ${MODULE_SRC_DIR}/dole_rsa.cc \
                  ${MODULE_SRC_DIR}/dole_logger.cc
MODULE_INCS     = ${TOPDIR}/dol/iris/test/storage \
                  ${TOPDIR}/dol/iris/test/offload \
                  ${TOPDIR}/nic/hal/third-party/grpc/include \
                  ${TOPDIR}/nic/hal/third-party/google/include \
                  ${TOPDIR}/nic/hal/third-party/openssl/include \
                  ${TOPDIR}/nic/hal/pd/capri
include ${MKDEFS}/post.mk
