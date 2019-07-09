/*
 *    Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t0_tcp_rx_k.h"

struct phv_ p;
struct s6_t0_tcp_rx_k_ k;
struct s6_t0_tcp_rx_dma_d d;

%%
    .param          tcp_slow_rx_write_serq_stage_start
    .param          tcp_rx_stats_stage_start
    .align

    /*
     * Global conditional variables
     * c7 drop packet and don't send to arm
     */
tcp_rx_dma_serq_stage_start:
    phvwr       p.s7_s2s_rx_stats_base, d.rx_stats_base
    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(tcp_rx_stats_stage_start)

    seq         c1, k.common_phv_write_arq, 1
    seq         c2, k.common_phv_write_serq, 0
    seq         c4, k.common_phv_skip_pkt_dma, 1
    seq         c5, k.common_phv_fatal_error, 1
    seq         c6, k.to_s6_payload_len, 0
    bcf         [c1 | c2 | c4 | c5 | c6], write_serq_check_more_slowly
    nop

dma_cmd_data:
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, TCP_PHV_RXDMA_COMMANDS_START
    bbeq        k.common_phv_ooq_tx2rx_pkt, 1, pkts_rcvd_stats_update_start

    /* Set the DMA_WRITE CMD for data */
    add         r3, k.to_s6_page, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)

    CAPRI_DMA_CMD_PKT2MEM_SETUP(pkt_dma_dma_cmd, r3, k.s1_s2s_payload_len)

dma_cmd_descr:
    /* Set the DMA_WRITE CMD for descr */
    add         r1, k.to_s6_descr, NIC_DESC_ENTRY_0_OFFSET

    addi        r3, r0, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)
    phvwr       p.aol_A0, k.{to_s6_page}.dx
    phvwr       p.aol_O0, r3.wx
    phvwr       p.aol_L0, k.{to_s6_payload_len}.wx

    CAPRI_DMA_CMD_PHV2MEM_SETUP(pkt_descr_dma_dma_cmd, r1, aol_A0, aol_next_pkt)
pkts_rcvd_stats_update_start:
    CAPRI_STATS_INC(pkts_rcvd, 1, d.pkts_rcvd, p.to_s7_pkts_rcvd)
pkts_rcvd_stats_update_end:

dma_tcp_flags:
    add         r1, k.to_s6_descr, NIC_DESC_ENTRY_TCP_FLAGS_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(tcp_flags_dma_dma_cmd, r1, tcp_app_header_flags, tcp_app_header_flags)

dma_cmd_serq_slot:
    CAPRI_OPERAND_DEBUG(k.to_s6_serq_pidx)
    // r5 = ring slot address
    sll         r5, k.to_s6_serq_pidx, d.nde_shift
    add         r1, r5, d.serq_base

    // r2 = descriptor address
    add         r2, k.to_s6_descr, d.nde_offset
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_BYPASS_BARCO, TCP_DDOL_BYPASS_BARCO
    phvwr.c1    p.ring_entry_len, k.to_s6_payload_len
    phvwr       p.ring_entry_descr_addr, r2

    // setup DMA to ring slot with descriptor address
    CAPRI_DMA_CMD_PHV2MEM_SETUP_WITH_LEN(ring_slot_dma_cmd, r1, ring_entry_pad, d.nde_len)

ring_doorbell:
    add         r1, k.to_s6_serq_pidx, 1
    and         r1, r1, d.consumer_num_slots_mask

    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET, d.consumer_qtype, d.consumer_lif) // r4 = addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE(tls_doorbell_dma_cmd, r4, db_data_pid, db_data_index)
    CAPRI_RING_DOORBELL_DATA(0, d.consumer_qid, d.consumer_ring, r1) // r3 = data
    phvwr.e     p.{db_data_pid...db_data_index}, r3.dx
    nop

write_serq_check_more_slowly:
    j           tcp_slow_rx_write_serq_stage_start
    nop

