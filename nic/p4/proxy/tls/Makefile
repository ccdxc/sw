
TOPDIR = ../../..
CUSTOM = my_all
include $(TOPDIR)/make/Makefile.inc

NCC = ../../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../../obj
GEN_DIR = ../../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi 
#TLS_PROJ = proxy
TLS_TX_PRE_CRYPTO_PROJ = tls_txdma_pre_crypto
TLS_TX_PRE_CRYPTO_GEN_FILE = $(GEN_DIR)/$(TLS_TX_PRE_CRYPTO_PROJ)/asm_out/ingress.h
TLS_TX_POST_CRYPTO_PROJ = tls_txdma_post_crypto
TLS_TX_POST_CRYPTO_GEN_FILE = $(GEN_DIR)/$(TLS_TX_POST_CRYPTO_PROJ)/asm_out/ingress.h

my_all: $(TLS_TX_PRE_CRYPTO_GEN_FILE) $(TLS_TX_POST_CRYPTO_GEN_FILE)

$(TLS_TX_PRE_CRYPTO_GEN_FILE): $(TLS_TX_PRE_CRYPTO_PROJ).p4
	$(NCC) $(NCC_OPTIONS) $(TLS_TX_PRE_CRYPTO_PROJ).p4 --gen-dir $(GEN_DIR)

$(TLS_TX_POST_CRYPTO_GEN_FILE): $(TLS_TX_POST_CRYPTO_PROJ).p4
	$(NCC) $(NCC_OPTIONS) $(TLS_TX_POST_CRYPTO_PROJ).p4 --gen-dir $(GEN_DIR)

