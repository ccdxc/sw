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
struct s3_t0_tcp_tx_k k;
struct s3_t0_tcp_tx_retx_d d;

%%
    .align
    .param          tcp_cc_and_fra_process_start
    .param          tcp_tx_read_nmdr_gc_idx_start

tcp_retx_process_start:
table_launch_cc_and_fra:
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_cc_and_fra_process_start, k.common_phv_qstate_addr,
                        TCP_TCB_CC_AND_FRA_OFFSET, TABLE_SIZE_512_BITS)

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
    add             r2, k.to_s3_addr, k.to_s3_offset
    phvwr           p.to_s5_addr, r2
    phvwr           p.to_s5_offset, k.to_s3_offset
    phvwr           p.to_s5_len, k.to_s3_len

    seq             c1, d.retx_tail_desc, r0
    bcf             [!c1], queue_to_tail
    nop

retx_empty:
    /*
     * retx empty, update head/tail/xmit desc and cursors
     */
    tblwr           d.retx_head_desc, k.to_s3_sesq_desc_addr
    tblwr           d.retx_tail_desc, k.to_s3_sesq_desc_addr
    tblwr           d.retx_next_desc, r0

    add             r2, k.to_s3_addr, k.to_s3_offset

    tblwr           d.retx_xmit_cursor, r2
    tblwr           d.retx_head_offset, k.to_s3_offset
    tblwr           d.retx_head_len, k.to_s3_len

    phvwr           p.to_s5_addr, r2
    phvwr           p.to_s5_offset, k.to_s3_offset
    phvwr           p.to_s5_len, k.to_s3_len
    tblwr           d.retx_snd_una, k.common_phv_snd_una
    nop
    nop.e
    nop

queue_to_tail:
    /*
     * If retx_tail is not NULL, queue to tail, update tail and return
     */
    add             r1, d.retx_tail_desc, NIC_DESC_ENTRY_NEXT_ADDR_OFFSET
    /*
     * k.to_s3_sesq_desc_addr is big endian, but descriptor contents are little
     * endian when passed via sesq. Use little endian format for next_addr as
     * well
     */
    memwr.wx        r1, k.to_s3_sesq_desc_addr
    tblwr           d.retx_tail_desc, k.to_s3_sesq_desc_addr
    seq             c1, d.retx_next_desc, r0
    tblwr.c1        d.retx_next_desc, k.to_s3_sesq_desc_addr
    nop.e
    nop


tcp_retx_snd_una_update:
    sub             r1, k.common_phv_snd_una, d.retx_snd_una
    slt             c1, r1, d.retx_head_len
    b.!c1           tcp_retx_snd_una_update_free_head
    tbladd          d.retx_snd_una, r1
    tblsub          d.retx_head_len, r1
    tbladd.e        d.retx_head_offset, r1
    tbladd          d.retx_xmit_cursor, r1

tcp_retx_snd_una_update_free_head:

    /*
     * We need to free d.retx_head_desc. Pass the address to read_nmdr_gc stage
     * to free it
     */
    phvwr           p.t1_s2s_free_desc_addr, d.retx_head_desc

    // write new descriptor address, offset and length
    tblwr           d.retx_head_desc, d.retx_next_desc
    tblwr           d.retx_next_desc, k.t0_s2s_next_addr
    tblwr           d.retx_head_offset, k.to_s3_offset
    tblwr           d.retx_head_len, k.to_s3_len
    add             r2, k.to_s3_addr, k.to_s3_offset
    tblwr           d.retx_xmit_cursor, r2
    
    phvwr           p.t0_s2s_packets_out_decr, 1
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

free_descriptor:
    CAPRI_NEXT_TABLE_READ_i(1, TABLE_LOCK_DIS, tcp_tx_read_nmdr_gc_idx_start,
                        TCP_NMDR_GC_IDX, TABLE_SIZE_32_BITS)
    nop.e
    nop

tcp_retx_retransmit:
    phvwr           p.t0_s2s_snd_nxt, d.retx_snd_una
    phvwr           p.to_s6_xmit_cursor_addr, d.retx_xmit_cursor
    phvwr           p.to_s6_xmit_cursor_len, d.retx_head_len
    phvwri          p.to_s6_pending_tso_data, 1
    nop.e
    nop
