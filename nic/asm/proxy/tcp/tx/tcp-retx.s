/*
 *    Implements the TX stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"

struct phv_ p;
struct s4_t0_tcp_tx_k k;
struct s4_t0_tcp_tx_retx_d d;

%%
    .align
    .param          tcp_cc_and_xmit_process_start
    .param          TNMDR_GC_TABLE_BASE
    .param          RNMDR_GC_TABLE_BASE

tcp_retx_process_start:
table_launch_cc_and_xmit:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_cc_and_xmit_process_start, k.common_phv_qstate_addr,
                        TCP_TCB_CC_AND_XMIT_OFFSET, TABLE_SIZE_512_BITS)

    seq             c1, k.common_phv_pending_snd_una_update, 1
    bcf             [c1], tcp_retx_snd_una_update

    seq             c1, k.common_phv_pending_rto, 1
    bcf             [c1], tcp_retx_retransmit

    seq             c1, k.common_phv_pending_sesq, 1
    seq.!c1         c1, k.common_phv_pending_asesq, 1

    b.c1            tcp_retx_enqueue
    nop

    nop.e
    nop


tcp_retx_enqueue:
    add             r2, k.to_s4_addr, k.to_s4_offset
    phvwr           p.to_s5_sesq_desc_addr, k.to_s4_sesq_desc_addr
    phvwr           p.to_s5_addr, r2
    phvwr           p.to_s5_offset, k.to_s4_offset
    phvwr           p.to_s5_len, k.to_s4_len

    seq             c1, d.retx_tail_desc, r0
    bcf             [!c1], queue_to_tail

retx_empty:
    /*
     * retx empty, update head/tail/xmit desc and cursors
     */
    tblwr           d.retx_head_desc, k.to_s4_sesq_desc_addr
    tblwr           d.retx_tail_desc, k.to_s4_sesq_desc_addr
    phvwr           p.to_s5_sesq_desc_addr, k.to_s4_sesq_desc_addr
    tblwr           d.retx_next_desc, r0

    add             r2, k.to_s4_addr, k.to_s4_offset

    tblwr           d.retx_xmit_cursor, r2
    tblwr           d.retx_head_offset, k.to_s4_offset
    tblwr           d.retx_head_len, k.to_s4_len

    phvwr           p.to_s5_addr, r2
    phvwr           p.to_s5_offset, k.to_s4_offset
    phvwr           p.to_s5_len, k.to_s4_len
    tblwr           d.retx_snd_una, k.common_phv_snd_una
    sne             c1, k.common_phv_debug_dol_free_rnmdr, r0
    bcf             [c1], free_rnmdr
    nop
    nop.e
    nop

queue_to_tail:
    /*
     * If retx_tail is not NULL, queue to tail, update tail and return
     */
    add             r1, d.retx_tail_desc, NIC_DESC_ENTRY_NEXT_ADDR_OFFSET
    memwr.w         r1, k.to_s4_sesq_desc_addr
    tblwr           d.retx_tail_desc, k.to_s4_sesq_desc_addr
    seq             c1, d.retx_next_desc, r0
    tblwr.c1        d.retx_next_desc, k.to_s4_sesq_desc_addr
    nop.e
    nop


free_rnmdr:
    // TODO: just for testing, fix this once retx is implemented
    sub             r3, k.to_s4_sesq_desc_addr, NIC_DESC_ENTRY_0_OFFSET
    phvwr           p.ring_entry_descr_addr, r3
    addui           r1, r0, hiword(RNMDR_GC_TABLE_BASE)
    addi            r1, r0, loword(RNMDR_GC_TABLE_BASE)
    add             r1, r1, RNMDR_GC_PRODUCER_TCP, RNMDR_GC_PER_PRODUCER_SHIFT
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry_dma_dma_cmd, r1, ring_entry_descr_addr, ring_entry_descr_addr)
    CAPRI_DMA_CMD_RING_DOORBELL2(doorbell_dma_dma_cmd, LIF_GC, 0,
                    CAPRI_HBM_GC_RNMDR_QID, CAPRI_RNMDR_GC_TCP_RING_PRODUCER,
                    0, db_data_pid, db_data_index)
    nop.e
    nop

free_tnmdr:
    // TODO: just for testing, fix this once retx is implemented
    sub             r3, k.to_s4_sesq_desc_addr, NIC_DESC_ENTRY_0_OFFSET
    phvwr           p.ring_entry_descr_addr, r3
    addui           r1, r0, hiword(TNMDR_GC_TABLE_BASE)
    addi            r1, r0, loword(TNMDR_GC_TABLE_BASE)
    add             r1, r1, TNMDR_GC_PRODUCER_TCP, TNMDR_GC_PER_PRODUCER_SHIFT
    CAPRI_DMA_CMD_PHV2MEM_SETUP(ringentry_dma_dma_cmd, r1, ring_entry_descr_addr, ring_entry_descr_addr)
    CAPRI_DMA_CMD_RING_DOORBELL2(doorbell_dma_dma_cmd, LIF_GC, 0,
                    CAPRI_HBM_GC_TNMDR_QID, CAPRI_TNMDR_GC_TCP_RING_PRODUCER,
                    0, db_data_pid, db_data_index)
    nop.e
    nop

tcp_clean_retx_queue:
    // TODO: schedule ourselves to clean retx
    jr              r7
    nop

tcp_tx_end_program:
    // We have no window, wait till window opens up
    CAPRI_CLEAR_TABLE_VALID(0)
    nop.e
    nop

tcp_retx_snd_una_update:
    sub             r1, k.common_phv_snd_una, d.retx_snd_una
    slt             c1, r1, d.retx_head_len
    b.!c1           tcp_retx_snd_una_update_free_head
    tbladd          d.retx_snd_una, r1
    tbladd.e        d.retx_head_offset, r1
    tblsub          d.retx_head_len, r1
    tbladd          d.retx_xmit_cursor, r1

tcp_retx_snd_una_update_free_head:
    // TODO : free d.retx_head_desc

    // write new descriptor address, offset and length
    tblwr           d.retx_head_desc, d.retx_next_desc
    tblwr           d.retx_next_desc, k.t0_s2s_next_addr
    tblwr           d.retx_head_offset, k.to_s4_offset
    tblwr           d.retx_head_len, k.to_s4_len
    add             r2, k.to_s4_addr, k.to_s4_offset
    tblwr           d.retx_xmit_cursor, r2
    
    // TODO : this needs to account for MTU
    //tblsub          d.packets_out, 1
    phvwr           p.t0_s2s_packets_out_decr, 1
    // This effectively cancels retransmission timer
    //tbladd          d.rto_pi, 1
    phvwr           p.t0_s2s_rto_pi_incr, 1
    phvwr           p.t0_s2s_pkts_acked, 1
    
    // If we have completely cleaned up, set tail to NULL
    seq             c1, d.retx_head_desc, r0
    tblwr.c1        d.retx_tail_desc, r0

    /*
     * if we still have more data to be cleaned up,
     * schedule ourselves again
     */
    sub             r1, k.common_phv_snd_una, d.retx_snd_una
    slt             c1, r1, d.retx_head_len
    // TODO : schedule again
    nop.e
    nop

tcp_retx_retransmit:
    phvwr           p.t0_s2s_snd_nxt, d.retx_snd_una
    phvwr           p.to_s6_xmit_cursor_addr, d.retx_xmit_cursor
    phvwr           p.to_s6_xmit_cursor_len, d.retx_head_len
    phvwri          p.to_s6_pending_tso_data, 1
    nop.e
    nop
