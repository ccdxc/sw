
#include "INGRESS_p.h"
#include "ingress.h"
#include "defines.h"
#include "../../p4/nw/include/defines.h"

struct phv_ p;
struct tx_table_s1_t0_k k;
struct tx_table_s1_t0_eth_tx_packet_d d;

#define ETH_DMA_CMD_PTR    (CAPRI_PHV_START_OFFSET(dma_hdr0_dma_cmd_type) / 16)

#define DMA_HEADERS(n) \
    phvwri      p.dma_hdr##n##_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_PKT; \
    phvwri      p.dma_hdr##n##_dma_cmd_cmdsize, 2; \
    phvwri      p.dma_hdr##n##_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(p4_intr_global_tm_iport); \
    phvwri      p.dma_hdr##n##_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(p4_intr_global_tm_instance_type); \
    phvwri      p.dma_hdr##n##_dma_cmd_phv_start_addr1, CAPRI_PHV_START_OFFSET(p4_txdma_intr_qid); \
    phvwri      p.dma_hdr##n##_dma_cmd_phv_end_addr1, CAPRI_PHV_END_OFFSET(p4_txdma_intr_txdma_rsv); \
    phvwri      p.dma_hdr##n##_dma_cmd_phv_start_addr2, CAPRI_PHV_START_OFFSET(eth_tx_app_hdr##n##_p4plus_app_id); \
    phvwri      p.dma_hdr##n##_dma_cmd_phv_end_addr2, CAPRI_PHV_END_OFFSET(eth_tx_app_hdr##n##_gso_valid);

#define DMA_PKT(n) \
    phvwri      p.dma_pkt##n##_dma_cmd_type, CAPRI_DMA_COMMAND_MEM_TO_PKT; \
    phvwri      p.dma_pkt##n##_dma_pkt_eop, 1; \
    phvwri      p.dma_pkt##n##_dma_cmd_host_addr, 1; \
    or          r1, d.addr_lo##n, d.addr_hi##n, 48; \
    add         r1, r0, r1.dx; \
    or          r1, r1[63:16], r1[11:8], 48; \
    phvwr       p.dma_pkt##n##_dma_cmd_addr, r1; \
    phvwr       p.dma_pkt##n##_dma_cmd_size, d.{len##n}.hx;

#define BUILD_APP_HEADER(n) \
    phvwri      p.eth_tx_app_hdr##n##_p4plus_app_id, P4PLUS_APPTYPE_CLASSIC_NIC; \
    seq         c1, d.vlan_insert##n##, 1; \
    addi.c1     r1, r0, P4PLUS_TO_P4_FLAGS_INSERT_VLAN_TAG; \
    phvwr.c1    p.eth_tx_app_hdr##n##_vlan_tag, d.{vlan_tci##n}.hx; \
    phvwr.c1    p.eth_tx_app_hdr##n##_flags, r1;

#define DEBUG_DESCR_FLD(name) \
    add         r7, r0, d.##name

#define DEBUG_DESCR(n) \
    DEBUG_DESCR_FLD(addr_lo##n); \
    DEBUG_DESCR_FLD(addr_hi##n); \
    DEBUG_DESCR_FLD(rsvd##n); \
    DEBUG_DESCR_FLD(num_sg_elems##n); \
    DEBUG_DESCR_FLD(opcode##n); \
    DEBUG_DESCR_FLD(len##n); \
    DEBUG_DESCR_FLD(vlan_tci##n); \
    DEBUG_DESCR_FLD(hdr_len_lo##n); \
    DEBUG_DESCR_FLD(hdr_len_hi##n); \
    DEBUG_DESCR_FLD(rsvd2##n); \
    DEBUG_DESCR_FLD(vlan_insert##n); \
    DEBUG_DESCR_FLD(cq_entry##n); \
    DEBUG_DESCR_FLD(csum##n); \
    DEBUG_DESCR_FLD(mss_or_csumoff_lo##n); \
    DEBUG_DESCR_FLD(mss_or_csumoff_hi##n); \
    DEBUG_DESCR_FLD(rsvd3_or_rsvd4##n)

%%

.align
eth_tx_packet:

#if 0
    DEBUG_DESCR(0)
    DEBUG_DESCR(1)
    DEBUG_DESCR(2)
    DEBUG_DESCR(3)
#endif

    // Set intrinsics
    phvwri      p.p4_intr_global_tm_iport, TM_PORT_DMA
    phvwri      p.p4_intr_global_tm_oport, TM_PORT_INGRESS

    // Setup DMA CMD PTR
    phvwri      p.p4_txdma_intr_dma_cmd_ptr, ETH_DMA_CMD_PTR

    // End of pipeline - Make sure no more tables will be launched
    phvwri      p.{app_header_table0_valid...app_header_table3_valid}, 0

    // R2 = Number of packets to dma
    add         r2, r0, k.eth_tx_t0_s2s_num_desc

    BUILD_APP_HEADER(0)
    DMA_HEADERS(0)
    DMA_PKT(0)

    subi        r2, r2, 1
    beq         r2, r0, eth_tx_packet_done
    nop

    BUILD_APP_HEADER(1)
    DMA_HEADERS(1)
    DMA_PKT(1)

    subi        r2, r2, 1
    beq         r2, r0, eth_tx_packet_done
    nop

    BUILD_APP_HEADER(2)
    DMA_HEADERS(2)
    DMA_PKT(2)

    subi        r2, r2, 1
    beq         r2, r0, eth_tx_packet_done
    nop

    BUILD_APP_HEADER(3)
    DMA_HEADERS(3)
    DMA_PKT(3)

eth_tx_packet_done:
    phvwri      p.dma_cmpl_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmpl_dma_cmd_host_addr, 1
    phvwr       p.dma_cmpl_dma_cmd_addr, k.eth_tx_t0_s2s_cq_desc_addr
    phvwri      p.dma_cmpl_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(eth_tx_cq_desc_status)
    phvwri      p.dma_cmpl_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(eth_tx_cq_desc_rsvd4)

    seq         c1, r0, k.eth_tx_t0_s2s_intr_assert_addr
    phvwri.e.c1 p.dma_cmpl_dma_cmd_eop, 1
    nop

    // DMA Interrupt
    phvwri      p.dma_intr_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwr       p.dma_intr_dma_cmd_addr, k.eth_tx_t0_s2s_intr_assert_addr
    phvwri      p.dma_intr_dma_cmd_phv_start_addr, CAPRI_PHV_START_OFFSET(eth_tx_t0_s2s_intr_assert_data)
    phvwri.e    p.dma_intr_dma_cmd_phv_end_addr, CAPRI_PHV_END_OFFSET(eth_tx_t0_s2s_intr_assert_data)
    phvwri      p.dma_intr_dma_cmd_eop, 1
