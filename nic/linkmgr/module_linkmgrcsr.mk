# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblinkmgrcsr.so
MODULE_PIPELINE = iris gft
MODULE_SRCS     = ${MODULE_SRC_DIR}/pknobs.cc \
                  ${MODULE_SRC_DIR}/cap_mx_api.cc \
                  ${MODULE_SRC_DIR}/cap_bx_api.cc \
                  ${MODULE_SRC_DIR}/cap_sbus_api.cc
MODULE_INCS     = ${TOPDIR}/nic/asic/capri/model/cap_top \
                  ${TOPDIR}/nic/asic/capri/model/cap_top/csr_defines \
                  ${TOPDIR}/nic/asic/capri/model/utils \
                  ${TOPDIR}/nic/asic/capri/model/cap_mx \
                  ${TOPDIR}/nic/asic/capri/model/cap_bx \
                  ${TOPDIR}/nic/asic/capri/model/cap_pcie \
                  ${TOPDIR}/nic/asic/capri/model/cap_ms \
                  ${TOPDIR}/nic/asic/ip/verif/pcpp \
                  ${TOPDIR}/nic/asic/capri/verif/apis \
                  ${TOPDIR}/nic/sdk/include/sdk/asic/capri \
                  ${TOPDIR}/nic/model_sim/include
MODULE_FLAGS    = ${NIC_CSR_FLAGS}
MODULE_DEFS     = -DCAPRI_SW ${NIC_CSR_DEFINES}
include ${MKDEFS}/post.mk
