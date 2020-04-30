# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET       = esp_v4_tunnel_n2h_rxdma.asmbin
MODULE_PREREQS      = ipsec_p4plus_esp_v4_tun_ntoh.p4bin
MODULE_PIPELINE     = iris gft
MODULE_INCS         = ${BLD_P4GEN_DIR}/esp_v4_tunnel_n2h_rxdma/asm_out \
                      ${BLD_P4GEN_DIR}/esp_v4_tunnel_n2h_rxdma/alt_asm_out \
                      ${MODULE_DIR}/../../include \
                      ${MODULE_DIR}/../../../common-p4+/include \
                      ${TOPDIR}/nic/p4/ipsec-p4+ \
                      ${TOPDIR}/nic/include \
                      ${MODULE_DIR}/../..
MODULE_DEPS         = $(shell find ${MODULE_DIR} -name '*.h')
MODULE_BIN_DIR      = ${BLD_BIN_DIR}/p4pasm
include ${MKDEFS}/post.mk
