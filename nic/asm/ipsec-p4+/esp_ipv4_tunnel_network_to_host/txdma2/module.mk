# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET       = esp_v4_tunnel_n2h_txdma2.asmbin
MODULE_PREREQS      = ipsec_p4plus_esp_v4_tun_ntoh.p4bin
MODULE_INCS         = ${BLD_GEN_DIR}/esp_v4_tunnel_n2h_txdma2/asm_out \
                      ${MODULE_DIR}/../../include \
                      ${MODULE_DIR}/../../../common-p4+/include \
                      ${TOPDIR}/nic/p4/ipsec-p4+ \
                      ${TOPDIR}/nic/include \
                      ${MODULE_DIR}/../.. \
                      ${TOPDIR}/nic/asm/proxy/tls/include \
                      ${TOPDIR}/nic/asm/cpu-p4plus/include
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
include ${MAKEDEFS}/post.mk
