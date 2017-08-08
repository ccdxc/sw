
CUSTOM = all
CUSTOM_CLEAN = clean

NCC = ../../tools/ncc/capri-ncc.py 
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --asm-out --p4-plus --no-ohi --pd-gen --gen-dir $(GEN_DIR)

all:
	$(NCC) $(NCC_OPTIONS) common_rxdma_actions.p4
	$(NCC) $(NCC_OPTIONS) common_txdma_actions.p4

clean:
	rm -rf $(GEN_DIR)/$(PROJ)

default:
	all
