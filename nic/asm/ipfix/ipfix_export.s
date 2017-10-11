#include "ipfix_defines.h"
#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "../common-p4+/include/capri-macros.h"

struct ipfix_export_packet_d d;
struct phv_                  p;

%%

ipfix_export_packet:
    phvwr       p.p4_txdma_intr_dma_cmd_ptr, \
                  CAPRI_PHV_START_OFFSET(phv2pkt_cmd1_dma_cmd_phv2pkt_pad) / 16

    // global intrinsic to packet
    phvwr       p.phv2pkt_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr       p.phv2pkt_cmd1_dma_cmd_phv_start_addr, \
                    CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
    phvwr       p.phv2pkt_cmd1_dma_cmd_phv_end_addr, \
                    CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)

    // p4plus header to packet
    phvwr       p.phv2pkt_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr       p.phv2pkt_cmd2_dma_cmd_phv_start_addr, \
                    CAPRI_PHV_START_OFFSET(app_header_app_type)
    phvwr       p.phv2pkt_cmd2_dma_cmd_phv_end_addr, \
                    CAPRI_PHV_START_OFFSET(app_header_app_type) + 14

    // memory to packet
    phvwr       p.mem2pkt_cmd_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr       p.mem2pkt_cmd_dma_cmd_addr, d.u.ipfix_export_packet_d.pktaddr
    phvwr       p.mem2pkt_cmd_dma_cmd_size, d.u.ipfix_export_packet_d.pktsize
    phvwr       p.mem2pkt_cmd_dma_cmd_eop, 1
    phvwr       p.mem2pkt_cmd_dma_pkt_eop, 1

    phvwr       p.app_header_table0_valid, 0
    phvwr       p.app_header_table1_valid, 0
    phvwr       p.app_header_table2_valid, 0
    phvwr.e     p.app_header_table3_valid, 0
    nop
