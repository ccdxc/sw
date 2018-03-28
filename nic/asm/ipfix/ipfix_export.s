#include "ipfix_defines.h"
#include "../../p4/iris/include/defines.h"
#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "../common-p4+/include/capri-macros.h"

struct ipfix_export_packet_d d;
struct phv_                  p;

%%

ipfix_export_packet:
    // exit if there are no records to be exported
    add         r1, d.{u.ipfix_export_packet_d.ipfix_hdr_offset}.hx, 16
    seq         c1, r1, d.{u.ipfix_export_packet_d.next_record_offset}.hx
    bcf         [c1], ipfix_export_packet_exit

    phvwr       p.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwr       p.p4_intr_global_tm_oport, TM_PORT_INGRESS
    phvwr       p.p4_intr_global_lif, 1003
    phvwri      p.ipfix_app_header_flags, \
                    (P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN | \
                     P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN)

    phvwr       p.p4_txdma_intr_dma_cmd_ptr, \
                  CAPRI_PHV_START_OFFSET(phv2pkt_cmd1_dma_cmd_phv2pkt_pad) / 16

    // global intrinsic to packet
    phvwr       p.phv2pkt_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT
    phvwr       p.phv2pkt_cmd1_dma_cmd_phv_start_addr, \
                    CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport)
    phvwr       p.phv2pkt_cmd1_dma_cmd_phv_end_addr, \
                    CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type)

    // txdma and p4plus header to packet
    phvwr       p.phv2pkt_cmd1_dma_cmd_phv_start_addr1, \
                    CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid)
    phvwr       p.phv2pkt_cmd1_dma_cmd_phv_end_addr1, \
                    CAPRI_PHV_END_OFFSET(ipfix_app_header_vlan_tag)
    phvwr       p.phv2pkt_cmd1_dma_cmd_cmdsize, 1

    // memory to packet
    phvwr       p.mem2pkt_cmd_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT
    phvwr       p.mem2pkt_cmd_dma_cmd_addr, d.u.ipfix_export_packet_d.pktaddr
    phvwr       p.mem2pkt_cmd_dma_cmd_size, \
                    d.{u.ipfix_export_packet_d.next_record_offset}.hx
    phvwr       p.mem2pkt_cmd_dma_cmd_eop, 1
    phvwr       p.mem2pkt_cmd_dma_pkt_eop, 1

    phvwr.e     p.{app_header_table0_valid...app_header_table2_valid}, 0
    nop

ipfix_export_packet_exit:
    phvwr.e     p.{app_header_table0_valid...app_header_table2_valid}, 0
    phvwr       p.p4_intr_global_drop, 1
