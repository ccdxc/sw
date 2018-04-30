TOPDIR = ../..
CUSTOM = my_all
CUSTOM_CLEAN = my_clean
include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --two-byte-profile
SMBDC_RESP_RXPROJ = smbdc_resp_rxdma
SMBDC_RESP_TXPROJ = smbdc_resp_txdma
SMBDC_REQ_RXPROJ = smbdc_req_rxdma
SMBDC_REQ_TXPROJ = smbdc_req_txdma
SMBDC_RESP_RXGEN_FILE = $(GEN_DIR)/$(SMBDC_RESP_RXPROJ)/asm_out/ingress.h
SMBDC_RESP_TXGEN_FILE = $(GEN_DIR)/$(SMBDC_RESP_TXPROJ)/asm_out/ingress.h
SMBDC_REQ_RXGEN_FILE = $(GEN_DIR)/$(SMBDC_REQ_RXPROJ)/asm_out/ingress.h
SMBDC_REQ_TXGEN_FILE = $(GEN_DIR)/$(SMBDC_REQ_TXPROJ)/asm_out/ingress.h
inc_deps = $(shell find ../include -name '*')
ncc_deps = $(shell find $(TOPDIR)/tools/ncc -path $(TOPDIR)/tools/ncc/utest -prune -o -name '*.c' -print -o -name '*.h' -print -o -name '*.py' -print)

#my_all: $(SMBDC_RESP_RXGEN_FILE) $(SMBDC_RESP_TXGEN_FILE) $(SMBDC_REQ_RXGEN_FILE) $(SMBDC_REQ_TXGEN_FILE)
my_all: $(SMBDC_REQ_TXGEN_FILE)

$(SMBDC_RESP_RXGEN_FILE): $(SMBDC_RESP_RXPROJ).p4 smbdc_rxdma_headers.p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(SMBDC_RESP_RXPROJ).p4 --gen-dir $(GEN_DIR)

$(SMBDC_RESP_TXGEN_FILE): $(SMBDC_RESP_TXPROJ).p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(SMBDC_RESP_TXPROJ).p4 --gen-dir $(GEN_DIR)

$(SMBDC_REQ_RXGEN_FILE): $(SMBDC_REQ_RXPROJ).p4 smbdc_rxdma_headers.p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(SMBDC_REQ_RXPROJ).p4 --gen-dir $(GEN_DIR)

$(SMBDC_REQ_TXGEN_FILE): $(SMBDC_REQ_TXPROJ).p4 $(ncc_deps)
	$(NCC) $(NCC_OPTIONS) $(SMBDC_REQ_TXPROJ).p4 --gen-dir $(GEN_DIR)

my_clean:
	$(RM) -fr $(GEN_DIR)/$(SMBDC_RESP_RXPROJ)
	$(RM) -fr $(GEN_DIR)/$(SMBDC_RESP_TXPROJ)
	$(RM) -fr $(GEN_DIR)/$(SMBDC_REQ_RXPROJ)
	$(RM) -fr $(GEN_DIR)/$(SMBDC_REQ_TXPROJ)
