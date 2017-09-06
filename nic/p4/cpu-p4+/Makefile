TOPDIR = ../..
CUSTOM = all
CUSTOM_CLEAN = clean

include $(TOPDIR)/make/Makefile.inc

NCC = ../../tools/ncc/capri-ncc.py
OBJ_DIR = ../../obj
GEN_DIR = ../../gen
NCC_OPTIONS = --p4-plus --pd-gen --asm-out --no-ohi --gen-dir $(GEN_DIR)

CPU_RX_PROJ = cpu_rxdma
CPU_TX_PROJ = cpu_txdma

all: 
	$(NCC) $(NCC_OPTIONS) $(CPU_RX_PROJ).p4 
	#$(NCC) $(NCC_OPTIONS) $(CPU_TX_PROJ).p4 

clean:
	rm -rf $(GEN_DIR)/$(CPU_RX_PROJ)
	#rm -rf $(GEN_DIR)/$(CPU_TX_PROJ)

default:
	all
