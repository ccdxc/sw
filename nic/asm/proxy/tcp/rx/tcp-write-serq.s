/*
 *    Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
    
struct phv_ p;
struct tcp_rx_write_serq_k k;
struct tcp_rx_write_serq_write_serq_d d;

%%
    .param          tcp_rx_stats_stage7_start
    .align    
tcp_rx_write_serq_stage6_start:
    CAPRI_CLEAR_TABLE0_VALID
    CAPRI_OPERAND_DEBUG(k.s6_s2s_debug_stage0_3_thread)
    CAPRI_OPERAND_DEBUG(k.s6_s2s_debug_stage4_7_thread)
    tblwr       d.debug_stage0_3_thread, k.s6_s2s_debug_stage0_3_thread
    tblwr       d.debug_stage4_7_thread, k.s6_s2s_debug_stage4_7_thread
    sne         c1, k.common_phv_write_arq, r0
    bcf         [c1], flow_write_serq_process_done
    nop
    /* r4 is loaded at the beginning of the stage with current timestamp value */
    tblwr       d.curr_ts, r4
    /* if (k.write_serq) is set in a previous stage , trigger writes to serq slot */
    sne         c1, k.common_phv_write_serq, r0
    bcf         [!c1], flow_write_serq_process_done
    nop

dma_cmd_data:
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, TCP_PHV_RXDMA_COMMANDS_START

    /* Set the DMA_WRITE CMD for data */
    add         r1, r0, k.to_s6_page
    addi        r3, r1, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)

    CAPRI_DMA_CMD_PKT2MEM_SETUP(dma_cmd0_dma_cmd, r3, k.to_s6_payload_len)

dma_cmd_descr:    
    /* Set the DMA_WRITE CMD for descr */
    add         r5, k.to_s6_descr, r0
    addi        r1, r5, NIC_DESC_ENTRY_0_OFFSET
        
    phvwr       p.aol_A0, k.{to_s6_page}.dx
    addi        r3, r0, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    phvwr       p.aol_O0, r3.wx
    phvwr       p.aol_L0, k.{to_s6_payload_len}.wx
    phvwr       p.aol_A1, r0
    phvwr       p.aol_O1, r0
    phvwr       p.aol_L1, r0
    phvwr       p.aol_A2, r0
    phvwr       p.aol_O2, r0
    phvwr       p.aol_L2, r0
    phvwr       p.aol_next_addr, r0

    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd1_dma_cmd, r1, aol_A0, aol_next_pkt)
    addi        r7, r0, 1
    

    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_SERQ, TCP_DDOL_DONT_QUEUE_TO_SERQ
    bcf         [c1], dma_cmd_write_rx2tx_shared
    nop
dma_cmd_serq_slot:
    add         r5, r0, k.to_s6_xrq_pidx
    sll         r5, r5, NIC_SERQ_ENTRY_SIZE_SHIFT
    /* Set the DMA_WRITE CMD for SERQ slot */
    add         r1, r5, k.to_s6_xrq_base

    phvwr       p.ring_entry_descr_addr, k.to_s6_descr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd2_dma_cmd, r1, ring_entry_descr_addr, ring_entry_descr_addr)
    addi        r7, r7, 1
dma_cmd_write_rx2tx_shared:
    /* Set the DMA_WRITE CMD for copying rx2tx shared data from phv to mem */
    seq         c1, k.common_phv_pending_txdma, 1
    bcf         [!c1], tcp_serq_produce
    nop
    add         r5, TCP_TCB_RX2TX_SHARED_WRITE_OFFSET, k.common_phv_qstate_addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd3_dma_cmd, r5, rx2tx_snd_una, rx2tx_pad1_rx2tx)
    addi        r7, r7, 1

dma_cmd_write_rx2tx_extra_shared:
    /* Set the DMA_WRITE CMD for copying rx2tx extra shared data from phv to mem */
    add         r5, TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, k.common_phv_qstate_addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP(dma_cmd4_dma_cmd, r5, rx2tx_extra_rcv_mss, rx2tx_extra__padding)
    addi        r7, r7, 1

dma_cmd_ring_tcp_tx_doorbell:
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DONT_RING_TX_DOORBELL, TCP_DDOL_DONT_RING_TX_DOORBELL
    bcf         [c1], tcp_serq_produce

    CAPRI_DMA_CMD_RING_DOORBELL2(dma_cmd5_dma_cmd, LIF_TCP, 0,k.common_phv_fid, TCP_SCHED_RING_PENDING,
                                 0, db_data2_pid, db_data2_index)

    addi        r7, r7, 1

tcp_serq_produce:
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_PKT_TO_SERQ, TCP_DDOL_PKT_TO_SERQ
    smeqb       c2, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_SERQ, TCP_DDOL_DONT_QUEUE_TO_SERQ
    bcf         [!c1 & !c2], ring_doorbell
    nop
    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd5_dma_cmd)
    b           flow_write_serq_process_done
    nop
ring_doorbell:

    CAPRI_DMA_CMD_RING_DOORBELL2(dma_cmd6_dma_cmd, LIF_TLS, 0, k.common_phv_fid, 0,
                                 k.to_s6_xrq_pidx, db_data_pid, db_data_index)

    CAPRI_DMA_CMD_STOP_FENCE(dma_cmd6_dma_cmd)
    addi        r7, r7, 1
    
flow_write_serq_process_done:
stats:

    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_TEST_ATOMIC_STATS, TCP_DDOL_TEST_ATOMIC_STATS
    bcf         [!c1], pkts_rcvd_stats_update_start
    nop
    // Debug: Force increment of atomic stats
debug_pkts_rcvd_stats_update_start:
    phvwr       p.to_s7_pkts_rcvd, 1
    phvwr       p.to_s7_pages_alloced, 1
    phvwr       p.to_s7_desc_alloced, 1
    phvwr       p.to_s7_debug_num_phv_to_mem, r7
    phvwr       p.to_s7_debug_num_pkt_to_mem, 1
    // End debug stats increment, skip regular stats update
    b           tcp_write_serq_stats_end
    nop

    // Non-debug stats increment
pkts_rcvd_stats_update_start:
    CAPRI_STATS_INC(pkts_rcvd, 8, 1, d.pkts_rcvd)
pkts_rcvd_stats_update:
    CAPRI_STATS_INC_UPDATE(1, d.pkts_rcvd, p.to_s7_pkts_rcvd)
pkts_rcvd_stats_update_end:

pages_alloced_stats_update_start:
    CAPRI_STATS_INC(pages_alloced, 8, 1, d.pages_alloced)
pages_alloced_stats_update:
    CAPRI_STATS_INC_UPDATE(1, d.pages_alloced, p.to_s7_pages_alloced)
pages_alloced_stats_update_end:

desc_alloced_stats_update_start:
    CAPRI_STATS_INC(desc_alloced, 8, 1, d.desc_alloced)
desc_alloced_stats_update:
    CAPRI_STATS_INC_UPDATE(1, d.desc_alloced, p.to_s7_desc_alloced)
desc_alloced_stats_update_end:

debug_num_phv_to_mem_stats_update_start:
    CAPRI_STATS_INC(debug_num_phv_to_mem, 8, r7, d.debug_num_phv_to_mem)
debug_num_phv_to_mem_stats_update:
    CAPRI_STATS_INC_UPDATE(r7, d.debug_num_phv_to_mem, p.to_s7_debug_num_phv_to_mem)
debug_num_phv_to_mem_stats_update_end:

debug_num_pkt_to_mem_stats_update_start:
    CAPRI_STATS_INC(debug_num_pkt_to_mem, 8, 1, d.debug_num_pkt_to_mem)
debug_num_pkt_to_mem_stats_update:
    CAPRI_STATS_INC_UPDATE(1, d.debug_num_pkt_to_mem, p.to_s7_debug_num_pkt_to_mem)
debug_num_pkt_to_mem_stats_update_end:
tcp_write_serq_stats_end:

    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(tcp_rx_stats_stage7_start)
    nop.e
    nop

