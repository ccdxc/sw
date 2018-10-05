# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = liblinkmgr_capricsr_int.so
MODULE_SRCS     = ${TOPDIR}/nic/asic/capri/model/cap_bx/cap_bx_csr.cc \
                  ${TOPDIR}/nic/asic/capri/model/cap_mx/cap_mx_csr.cc \
                  ${TOPDIR}/nic/asic/capri/model/utils/cap_csr_base.cc \
                  ${TOPDIR}/nic/asic/ip/verif/pcpp/pen_csr_base.cc \
                  ${TOPDIR}/nic/asic/ip/verif/pcpp/LogMsg.cc \
                  ${TOPDIR}/nic/asic/ip/verif/pcpp/msg_man.cc \
                  ${TOPDIR}/nic/asic/ip/verif/pcpp/msg_stream.cc \
                  ${TOPDIR}/nic/asic/capri/model/utils/cap_csr_py_if.cc \
                  ${TOPDIR}/nic/asic/ip/verif/pcpp/cpu.cc \
                  ${TOPDIR}/nic/asic/ip/verif/pcpp/pknobs.cc
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
