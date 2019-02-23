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
    .align

    /*
     * Global conditional variables
     * c7 drop packet and don't send to arm
     */
tcp_slow_rx_write_serq_stage_start:
    seq         c1, k.common_phv_write_arq, 1
    seq         c2, k.common_phv_write_serq, 1
    setcf       c7, [!c1 & !c2]
    seq         c4, k.common_phv_skip_pkt_dma, 1
    setcf       c7, [c7 | c4]
    seq         c3, k.common_phv_fatal_error, 1
    bcf         [c7 | c3], slow_flow_write_serq_drop
    nop
    bcf         [c1], slow_flow_write_serq_process_done
    nop


dma_slow_cmd_data:
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, TCP_PHV_RXDMA_COMMANDS_START

    /*
     * If payload len is 0, skip DMA of page
     */
    seq         c1, k.to_s6_payload_len, 0
    b.c1        dma_slow_cmd_descr

    seq         c1, k.common_phv_skip_pkt_dma, 1
    b.c1        dma_slow_cmd_data_skip

    /* Set the DMA_WRITE CMD for data */
    add         r3, k.to_s6_page, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)

    CAPRI_DMA_CMD_PKT2MEM_SETUP(pkt_dma_dma_cmd, r3, k.s6_s2s_payload_len)
    b           dma_slow_cmd_descr
    nop

dma_slow_cmd_data_skip:
    CAPRI_DMA_CMD_SKIP_SETUP(pkt_dma_skip_dma_cmd)

dma_slow_cmd_descr:
    /*
     * If write_serq is 0, don't DMA to descriptor (we could get here for
     * l7 proxy redirect case, in which case, l7 asm file has DMA instructions
     * to l7 descr)
     */
    seq         c1, k.common_phv_write_serq, 1
    bcf         [!c1], slow_flow_write_serq_process_done 
    nop

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

dma_slow_tcp_flags:
    add         r1, k.to_s6_descr, NIC_DESC_ENTRY_TCP_FLAGS_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(tcp_flags_dma_dma_cmd, r1, tcp_app_header_flags, tcp_app_header_flags)
dma_slow_cmd_serq_slot:
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

tcp_slow_serq_produce:
    seq         c1, k.common_phv_write_serq, 1
    bcf         [!c1], slow_flow_write_serq_process_done
    nop

slow_ring_doorbell:
    add         r1, k.to_s6_serq_pidx, 1
    and         r1, r1, d.consumer_num_slots_mask

    CAPRI_RING_DOORBELL_ADDR(0, DB_IDX_UPD_PIDX_SET, DB_SCHED_UPD_SET, 0, d.consumer_lif) // r4 = addr
    CAPRI_DMA_CMD_PHV2MEM_SETUP_STOP_FENCE(tls_doorbell_dma_cmd, r4, db_data_pid, db_data_index)
    CAPRI_RING_DOORBELL_DATA(0, d.consumer_qid, d.consumer_ring, r1) // r3 = data
    phvwr.e     p.{db_data_pid...db_data_index}, r3.dx
    nop

slow_flow_write_serq_process_done:
    nop.e
    nop

slow_flow_write_serq_drop:
    sne         c1, k.common_phv_pending_txdma, 0
    phvwri.!c1  p.p4_intr_global_drop, 1
    b.!c1       slow_flow_write_serq_process_done
    nop
    phvwri.c1   p.p4_rxdma_intr_dma_cmd_ptr, TCP_PHV_RXDMA_COMMANDS_START
    seq         c2, k.to_s6_payload_len, 0
    setcf       c3, [c1 & c2]
    b.c3        dma_slow_cmd_descr
    nop
    b.c1        dma_slow_cmd_data_skip
    nop
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, (CAPRI_PHV_START_OFFSET(cpu_hdr_dma_dma_cmd_type) / 16)
    nop
