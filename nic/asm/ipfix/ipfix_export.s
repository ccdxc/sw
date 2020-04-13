#include "ipfix_defines.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"
#include "ipfix/asm_out/INGRESS_p.h"
#include "ipfix/asm_out/ingress.h"
#include "../common-p4+/include/capri-macros.h"

struct ipfix_export_packet_k k;
struct ipfix_export_packet_d d;
struct phv_                  p;

%%
    .param      ipfix_stats_base

// cmd1 <-- phv2pkt (if packet needs to be sent)
// cmd2 <-- mem2pkt (if packet needs to be sent)
// cmd3 <-- disable doorbell self (set cmd_eop if scan is complete)
// cmd4 <-- update table type and table index for next scan
// cmd5 <-- ring doorbell self-16 for next scan

ipfix_export_packet:
    addi        r5, r0, loword(ipfix_stats_base)
    addui       r5, r5, hiword(ipfix_stats_base)
    sub         r1, k.ipfix_metadata_export_id, IPFIX_EXPORT_ID_MAX
    add         r5, r5, r1, 6
    add         r5, r5, IPFIX_STATS_NUM_EXPORTED_BYTES_OFFSET

    phvwr       p.p4_txdma_intr_dma_cmd_ptr, \
                    CAPRI_PHV_START_OFFSET(phv2mem_cmd1_dma_cmd_pad) / 16

    // exit if there are no records to be exported
    add         r1, d.{u.ipfix_export_packet_d.ipfix_hdr_offset}.hx, 16
    seq         c1, r1, d.{u.ipfix_export_packet_d.next_record_offset}.hx
    bcf         [c1], ipfix_export_packet_exit
    phvwr.c1    p.phv2mem_cmd4_dma_cmd_eop, 1

    // is the scan complete?
    seq         c1, k.ipfix_metadata_scan_complete, 1
    bcf         [c1], ipfix_export_dma_packet
    phvwr.c1    p.phv2mem_cmd4_dma_cmd_eop, 1

    // ring doorbell (self-16)
    addi        r1, r0, DB_ADDR_BASE
    or          r1, r1, 0x3, DB_UPD_SHFT
    or          r1, r1, LIF_IPFIX, DB_LIF_SHFT
    sub         r2, k.ipfix_metadata_export_id, IPFIX_EXPORT_ID_MAX
    add         r2, r0, r2, 24
    phvwr       p.ipfix_s5_metadata_doorbell2_data, r2.dx
    phvwr       p.phv2mem_cmd5_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr       p.phv2mem_cmd5_dma_cmd_phv_start_addr, \
                    CAPRI_PHV_START_OFFSET(ipfix_s5_metadata_doorbell2_data)
    phvwr       p.phv2mem_cmd5_dma_cmd_phv_end_addr, \
                    CAPRI_PHV_END_OFFSET(ipfix_s5_metadata_doorbell2_data)
    phvwr       p.phv2mem_cmd5_dma_cmd_addr, r1
    phvwr       p.phv2mem_cmd5_dma_cmd_wr_fence, 1
    phvwr       p.phv2mem_cmd5_dma_cmd_eop, 1

ipfix_export_dma_packet:
    // update exporter stats
    addi        r6, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    add         r6, r6, r5[26:0]
    addi        r1, r0, 0x1000001
    or          r7, d.{u.ipfix_export_packet_d.next_record_offset}.hx, r1, 32
    or          r7, r7, r5[31:27], 58
    memwr.dx    r6, r7

    phvwr       p.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwr       p.p4_intr_global_tm_oport, TM_PORT_INGRESS
    phvwr       p.p4_intr_global_lif, LIF_CPU
    phvwr       p.ipfix_app_header_p4plus_app_id, P4PLUS_APPTYPE_TELEMETRY
    phvwri      p.ipfix_app_header_flags, \
                    (P4PLUS_TO_P4_FLAGS_UPDATE_IP_LEN | \
                     P4PLUS_TO_P4_FLAGS_UPDATE_UDP_LEN)

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
    phvwr       p.mem2pkt_cmd_dma_cmd_addr, d.{u.ipfix_export_packet_d.pktaddr}.dx
    phvwr       p.mem2pkt_cmd_dma_cmd_size, \
                    d.{u.ipfix_export_packet_d.next_record_offset}.hx
    phvwr       p.mem2pkt_cmd_dma_cmd_pkt_eop, 1

ipfix_export_packet_exit:
    // disable doorbell (self)
    addi        r1, r0, DB_ADDR_BASE
    or          r1, r1, 0x2, DB_UPD_SHFT
    or          r1, r1, LIF_IPFIX, DB_LIF_SHFT
    add         r2, r0, k.ipfix_metadata_export_id, 24
    phvwr       p.ipfix_s5_metadata_doorbell1_data, r2.dx
    phvwr       p.phv2mem_cmd3_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr       p.phv2mem_cmd3_dma_cmd_phv_start_addr, \
                    CAPRI_PHV_START_OFFSET(ipfix_s5_metadata_doorbell1_data)
    phvwr       p.phv2mem_cmd3_dma_cmd_phv_end_addr, \
                    CAPRI_PHV_END_OFFSET(ipfix_s5_metadata_doorbell1_data)
    phvwr       p.phv2mem_cmd3_dma_cmd_addr, r1

    // update table type and next index in qstate of (self-16)
    sub         r1, k.{ipfix_metadata_qstate_addr_sbit0_ebit1,\
                       ipfix_metadata_qstate_addr_sbit2_ebit33}, ((64*16)-32)
    phvwr       p.phv2mem_cmd4_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr       p.phv2mem_cmd4_dma_cmd_phv_start_addr, \
                    CAPRI_PHV_START_OFFSET(ipfix_t1_metadata_next_record_offset)
    phvwr       p.phv2mem_cmd4_dma_cmd_phv_end_addr, \
                    CAPRI_PHV_END_OFFSET(ipfix_t1_metadata_flow_hash_index_next)
    phvwr.e     p.phv2mem_cmd4_dma_cmd_addr, r1

    phvwr       p.{app_header_table0_valid...app_header_table2_valid}, 0
