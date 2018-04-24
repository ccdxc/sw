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
struct s6_t0_tcp_rx_write_serq_d d;

%%
    .align

    /*
     * Global conditional variables
     * c7 drop packet and don't send to arm
     */
tcp_slow_rx_write_serq_stage_start:
    seq         c1, k.common_phv_write_arq, 1
    seq         c2, k.common_phv_write_serq, 1
    seq         c3, k.common_phv_l7_proxy_type_redirect, 1
    setcf       c7, [!c1 & !c2 & !c3]
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
    add         r1, k.to_s6_page, k.to_s6_ooo_offset
    addi        r3, r1, (NIC_PAGE_HDR_SIZE + NIC_PAGE_HEADROOM)

    /*
     * For SACK case, to_s6_payload_len is total accumulated length of
     * packet. s6_s2s_payload_len is the packet we just got and that
     * is what we want to DMA
     */
    CAPRI_DMA_CMD_PKT2MEM_SETUP(pkt_dma_dma_cmd, r3, k.s6_s2s_payload_len)
    seq         c1, k.common_phv_ooo_rcv, 1
    seq         c2, k.common_phv_pending_txdma, 0
    setcf       c1, [c1 & c2]
    phvwr.c1    p.pkt_dma_dma_cmd_eop, 1
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

    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_SERQ, TCP_DDOL_DONT_QUEUE_TO_SERQ
    bcf         [c1], tcp_slow_serq_produce
    nop
dma_slow_tcp_flags:
    add         r1, k.to_s6_descr, NIC_DESC_ENTRY_TCP_FLAGS_OFFSET
    CAPRI_DMA_CMD_PHV2MEM_SETUP(tcp_flags_dma_dma_cmd, r1, tcp_app_header_flags, tcp_app_header_flags)
dma_slow_cmd_serq_slot:
    CAPRI_OPERAND_DEBUG(k.to_s6_serq_pidx)
    sll         r5, k.to_s6_serq_pidx, NIC_SERQ_ENTRY_SIZE_SHIFT
    /* Set the DMA_WRITE CMD for SERQ slot */
    add         r1, r5, d.serq_base
    // increment serq pi as a part of ringing dorrbell

    phvwr       p.ring_entry_descr_addr, k.to_s6_descr
    /* The new SERQ defintion includes the descriptor pointer followed by the first AOL */
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ring_slot_dma_cmd, r1, ring_entry_descr_addr, aol_L0)

tcp_slow_serq_produce:
    seq         c1, k.common_phv_write_serq, 1
    bcf         [!c1], slow_flow_write_serq_process_done
    nop
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_PKT_TO_SERQ, TCP_DDOL_PKT_TO_SERQ
    smeqb       c2, k.common_phv_debug_dol, TCP_DDOL_DONT_QUEUE_TO_SERQ, TCP_DDOL_DONT_QUEUE_TO_SERQ
    bcf         [!c1 & !c2], slow_ring_doorbell
    nop
    b           slow_flow_write_serq_process_done
    nop

slow_ring_doorbell:
    seq         c1, k.common_phv_l7_proxy_en, 1
    bcf         [!c1], slow_ring_doorbell_no_proxy
    add         r1, k.to_s6_serq_pidx, 1

slow_ring_doorbell_proxy:
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI(tls_doorbell_dma_cmd, LIF_TLS, 0,
                                 k.common_phv_fid, TLS_SCHED_RING_SERQ,
                                 r1, db_data_pid, db_data_index)
    b slow_flow_write_serq_process_done
    nop

slow_ring_doorbell_no_proxy:
    CAPRI_DMA_CMD_RING_DOORBELL2_SET_PI_STOP_FENCE(tls_doorbell_dma_cmd, LIF_TLS, 0,
                                 k.common_phv_fid, TLS_SCHED_RING_SERQ,
                                 r1, db_data_pid, db_data_index)

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
    phvwri      p.p4_rxdma_intr_dma_cmd_ptr, (CAPRI_PHV_START_OFFSET(rx2tx_or_cpu_hdr_dma_dma_cmd_type) / 16)
    nop
