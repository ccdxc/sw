# {C} Copyright 2018 Pensando Systems Inc. All rights reserved

include ${MKDEFS}/pre.mk
MODULE_TARGET   = libhal_src.so
MODULE_PIPELINE = iris gft
MODULE_SRCS     := ${MODULE_SRC_DIR}/hal.cc \
                   ${MODULE_SRC_DIR}/hal_state.cc \
                   ${MODULE_SRC_DIR}/hal_obj.cc \
                   ${MODULE_SRC_DIR}/hal_trace.cc \
                   $(wildcard ${MODULE_SRC_DIR}/src/stats/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/src/debug/*.cc) \
                   $(wildcard ${MODULE_SRC_DIR}/src/utils/*.cc)
ifeq ($(ARCH),x86_64)
MODULE_SRCS     := ${MODULE_SRCS} \
                   $(wildcard ${MODULE_SRC_DIR}/src/internal/*.cc)
else
MODULE_SRCS     := ${MODULE_SRCS} \
                   ${MODULE_SRC_DIR}/src/internal/barco_rings.cc \
                   ${MODULE_SRC_DIR}/src/internal/cpulif.cc \
                   ${MODULE_SRC_DIR}/src/internal/cpucb.cc \
                   ${MODULE_SRC_DIR}/src/internal/internal.cc \
                   ${MODULE_SRC_DIR}/src/internal/event.cc \
                   ${MODULE_SRC_DIR}/src/internal/eth.cc \
                   ${MODULE_SRC_DIR}/src/internal/descriptor_aol.cc \
                   ${MODULE_SRC_DIR}/src/internal/crypto_keys.cc \
                   ${MODULE_SRC_DIR}/src/internal/crypto_apis.cc \
                   ${MODULE_SRC_DIR}/src/internal/proxy.cc \
                   ${MODULE_SRC_DIR}/src/internal/ipseccb.cc \
                   ${MODULE_SRC_DIR}/src/internal/swphv.cc \
                   ${MODULE_SRC_DIR}/src/internal/quiesce.cc \
                   ${MODULE_SRC_DIR}/src/internal/wring.cc \
                   ${MODULE_SRC_DIR}/src/internal/tlscb.cc \
                   ${MODULE_SRC_DIR}/src/internal/tcpcb.cc \
                   ${MODULE_SRC_DIR}/src/internal/rawccb.cc \
                   ${MODULE_SRC_DIR}/src/internal/proxyrcb.cc \
                   ${MODULE_SRC_DIR}/src/internal/proxyccb.cc \
                   ${MODULE_SRC_DIR}/src/internal/rawrcb.cc \
                   ${MODULE_SRC_DIR}/src/internal/system.cc
endif
MODULE_INCS     = ${TOPDIR}/nic/hal/third-party/openssl/include \
                  ${TOPDIR}/nic/hal/iris/delphi \
                  ${BLD_PROTOGEN_DIR}/
include ${MKDEFS}/post.mk
