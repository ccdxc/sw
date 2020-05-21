# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MKDEFS}/pre.mk
MODULE_TARGET   = ipsec_p4plus_esp_v4_tun_hton.p4bin
MODULE_PIPELINE = iris gft apulu
MODULE_SRCS     = ${MODULE_SRC_DIR}/esp_ipv4_tunnel_h2n_rxdma.p4 \
                  ${MODULE_SRC_DIR}/esp_ipv4_tunnel_h2n_txdma1.p4 \
                  ${MODULE_SRC_DIR}/esp_ipv4_tunnel_h2n_txdma2.p4
ifeq ($(ASIC),elba)
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi --asic elba \
                  --two-byte-profile --fe-flags="-DELBA -I${TOPDIR} -I${SDKDIR}" \
                  --gen-dir ${BLD_P4GEN_DIR}
else
MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi \
                  --two-byte-profile --fe-flags="-I${TOPDIR} -I${SDKDIR}" \
                  --gen-dir ${BLD_P4GEN_DIR}
endif
MODULE_DEPS     = $(shell find ${MODULE_DIR} -name '*.p4' -o -name '*.h') \
                  $(shell find ${MODULE_DIR}/../ -name '*.p4' -o -name '*.h') \
                  $(shell find ${MODULE_DIR}/../../include -name '*') \
                  $(shell find ${MODULE_DIR}/../../common-p4+ -name '*.p4')
include ${MKDEFS}/post.mk
