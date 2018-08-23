# {C} Copyright 2018 Pensando Systems Inc. All rights reserved
include ${MAKEDEFS}/pre.mk
MODULE_TARGET   = tls.p4bin

MODULE_SRCS     = tls_txdma_pre_crypto_enc.p4 \
                  tls_txdma_pre_crypto_dec.p4 \
                  tls_txdma_pre_crypto_mac.p4 \
                  tls_txdma_post_crypto_enc.p4 \
                  tls_txdma_post_crypto_dec.p4 \
                  tls_txdma_pre_crypto_dec_aesgcm_newseg.p4 \
                  tls_txdma_post_crypto_dec_aesgcm_newseg.p4 \
                  tls_txdma_post_crypto_mac.p4 \

MODULE_NCC_OPTS = --p4-plus --pd-gen --asm-out --no-ohi \
                  --two-byte-profile --fe-flags="-I${TOPDIR}" \
                  --gen-dir ${BLD_GEN_DIR}

MODULE_DEPS     = $(shell find ${MODULE_DIR}/ -name '*.p4' -o -name '*.h')
include ${MAKEDEFS}/post.mk
