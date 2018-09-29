# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblinkmgrcsr_helpers.so
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/cap_top_csr.cc \
                  ${MODULE_SRC_DIR}/cpu_hal_if.cc
MODULE_INCS     = ${TOPDIR}/nic/asic/capri/model/cap_top \
                  ${TOPDIR}/nic/asic/capri/model/cap_top/csr_defines \
                  ${TOPDIR}/nic/asic/capri/model/utils \
                  ${TOPDIR}/nic/asic/capri/model/cap_mx \
                  ${TOPDIR}/nic/asic/capri/model/cap_bx \
                  ${TOPDIR}/nic/asic/ip/verif/pcpp \
                  ${TOPDIR}/nic/asic/capri/verif/apis \
                  ${TOPDIR}/nic/model_sim/include
MODULE_FLAGS    = ${NIC_CSR_FLAGS} -Os
#-fvisibility=hidden
MODULE_DEFS     = ${NIC_CSR_DEFINES}
include ${MKDEFS}/post.mk
