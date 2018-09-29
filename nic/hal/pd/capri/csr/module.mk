# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = libcapri_csr_cpu_hal.so
MODULE_DEFS     = ${NIC_CSR_DEFINES}
MODULE_INCS     = ${NIC_CSR_INCS} \
                  ${TOPDIR}/nic/hal/third-party/openssl/include
MODULE_SRCS     = ${MODULE_SRC_DIR}/cpu_hal_if.cc
include ${MKDEFS}/post.mk
