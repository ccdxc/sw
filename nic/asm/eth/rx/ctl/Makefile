
TOPDIR = ../../../../
GENDIR = $(TOPDIR)/gen
OBJDIR = $(TOPDIR)/obj

CAPSIM = $(TOPDIR)/asic/capri/model/capsim-gen/bin/capsim
BINDIR = $(TOPDIR)/build/x86_64/iris/${ASIC}/bin/p4pasm
PROJ = eth_rxdma_actions
OBJ_DIR = $(TOPDIR)/obj/p4plus_bin
ASM_HDR_DIR = $(TOPDIR)/build/x86_64/iris/${ASIC}/gen/p4gen/$(PROJ)/asm_out/
ALT_ASM_HDR_DIR = $(TOPDIR)/build/x86_64/iris/${ASIC}/gen/p4gen/$(PROJ)/alt_asm_out/
INC_DIRS = -I. -I.. -I$(TOPDIR)/.. -I$(TOPDIR)/sdk -I$(ASM_HDR_DIR) -I$(ALT_ASM_HDR_DIR) -I$(TOPDIR)/asm/common-p4+/include -I$(TOPDIR)/p4/include -I$(TOPDIR)/include
SRC_DIR = .

OPTS = -d -b -e

eth_rx_app_header:
	$(CAPSIM) $(INC_DIRS) eth_rx_fetch_desc.ctl $(BINDIR)/eth_rx_fetch_desc.bin

eth_rx_rss_tcp4:
	$(CAPSIM) $(INC_DIRS) -P eth_rx_rss_skip=0xffffffff eth_rx_rss_tcp4.ctl $(BINDIR)/eth_rx_rss_params.bin

eth_rx_rss_udp4:
	$(CAPSIM) $(INC_DIRS) -P eth_rx_rss_skip=0xffffffff eth_rx_rss_udp4.ctl $(BINDIR)/eth_rx_rss_params.bin

eth_rx_rss_tcp6:
	$(CAPSIM) $(INC_DIRS) -P eth_rx_rss_skip=0xffffffff eth_rx_rss_tcp6.ctl $(BINDIR)/eth_rx_rss_params.bin

eth_rx_rss_udp6:
	$(CAPSIM) $(INC_DIRS) -P eth_rx_rss_skip=0xffffffff eth_rx_rss_udp6.ctl $(BINDIR)/eth_rx_rss_params.bin
