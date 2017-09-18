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
    
    phvwr       p.dma_cmd0_dma_cmd_addr, r3
    phvwr       p.dma_cmd0_dma_cmd_size, k.to_s6_payload_len
    
    phvwri      p.dma_cmd0_dma_cmd_type, CAPRI_DMA_COMMAND_PKT_TO_MEM
    phvwri      p.dma_cmd0_dma_cmd_eop, 0

dma_cmd_descr:    
    /* Set the DMA_WRITE CMD for descr */
    add         r5, k.to_s6_descr, r0
    addi        r1, r5, NIC_DESC_ENTRY_0_OFFSET
    phvwr       p.dma_cmd1_dma_cmd_addr, r1

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

    phvwri      p.dma_cmd1_dma_cmd_phv_start_addr, TCP_PHV_AOL_DESC_START
    phvwri      p.dma_cmd1_dma_cmd_phv_end_addr, TCP_PHV_AOL_DESC_END

    phvwri      p.dma_cmd1_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd1_dma_cmd_eop, 0
    addi        r7, r0, 1
    

    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_SERQ, TCP_DDOL_DONT_QUEUE_TO_SERQ
    bcf         [c1], dma_cmd_write_rx2tx_shared
    nop
dma_cmd_serq_slot:
    add         r5, r0, k.to_s6_serq_pidx
    sll         r5, r5, NIC_SERQ_ENTRY_SIZE_SHIFT
    /* Set the DMA_WRITE CMD for SERQ slot */
    add         r1, r5, k.to_s6_serq_base

    phvwr       p.dma_cmd2_dma_cmd_addr, r1
    phvwr       p.ring_entry_descr_addr, k.to_s6_descr
    CAPRI_OPERAND_DEBUG(k.to_s6_descr)
    phvwri      p.dma_cmd2_dma_cmd_phv_start_addr, TCP_PHV_RING_ENTRY_DESC_ADDR_START
    phvwri      p.dma_cmd2_dma_cmd_phv_end_addr, TCP_PHV_RING_ENTRY_DESC_ADDR_END
    phvwri      p.dma_cmd2_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd2_dma_cmd_eop, 0
    addi        r7, r7, 1
dma_cmd_write_rx2tx_shared:
    /* Set the DMA_WRITE CMD for copying rx2tx shared data from phv to mem */
    add         r5, TCP_TCB_RX2TX_SHARED_WRITE_OFFSET, k.common_phv_qstate_addr
    phvwr       p.dma_cmd3_dma_cmd_addr, r5
    phvwri      p.dma_cmd3_dma_cmd_phv_start_addr, TCP_PHV_RX2TX_SHARED_START
    phvwri      p.dma_cmd3_dma_cmd_phv_end_addr, TCP_PHV_RX2TX_SHARED_END
    phvwri      p.dma_cmd3_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd3_dma_cmd_eop, 0
    addi        r7, r7, 1

dma_cmd_write_rx2tx_extra_shared:
    /* Set the DMA_WRITE CMD for copying rx2tx extra shared data from phv to mem */
    add         r5, TCP_TCB_RX2TX_SHARED_EXTRA_OFFSET, k.common_phv_qstate_addr
    phvwr       p.dma_cmd4_dma_cmd_addr, r5
    phvwri      p.dma_cmd4_dma_cmd_phv_start_addr, TCP_PHV_RX2TX_SHARED_EXTRA_START
    phvwri      p.dma_cmd4_dma_cmd_phv_end_addr, TCP_PHV_RX2TX_SHARED_EXTRA_END
    phvwri      p.dma_cmd4_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM
    phvwri      p.dma_cmd4_dma_cmd_eop, 0
    addi        r7, r7, 1

tcp_serq_produce:
    sne         c1, k.common_phv_debug_dol, r0
    bcf         [!c1], ring_doorbell
    nop
    phvwri      p.dma_cmd4_dma_cmd_eop, 1
    phvwri      p.dma_cmd4_dma_cmd_wr_fence, 1
    b           flow_write_serq_process_done
    nop
ring_doorbell:
    /* address will be in r4 */
    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_INC, DB_SCHED_UPD_SET, 0, LIF_TLS)
    /* data will be in r3 */
    CAPRI_RING_DOORBELL_DATA(0, k.common_phv_fid, 0, k.to_s6_serq_pidx)
    
    phvwr       p.dma_cmd5_dma_cmd_addr, r4
    phvwr       p.{db_data_pid...db_data_index}, r3.dx

    phvwri      p.dma_cmd5_dma_cmd_phv_start_addr, TCP_PHV_DB_DATA_START
    phvwri      p.dma_cmd5_dma_cmd_phv_end_addr, TCP_PHV_DB_DATA_END
    phvwri      p.dma_cmd5_dma_cmd_type, CAPRI_DMA_COMMAND_PHV_TO_MEM

    phvwri      p.dma_cmd5_dma_cmd_eop, 1
    phvwri      p.dma_cmd5_dma_cmd_wr_fence, 1
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
    b           desc_alloced_stats_update_end
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

    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(tcp_rx_stats_stage7_start)
    nop.e
    nop

