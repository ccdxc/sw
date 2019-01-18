/*
 *  TCP ACK processing (ack received from peer)
 *  
 *  Update snd_una, and inform tx pipeline
 *  
 *  Handle dup_acks and fast retransmissions
 *  
 *  Handle window change, and inform tx pipeline
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"  
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s2_t0_tcp_rx_k.h"
    
struct phv_ p;
struct s2_t0_tcp_rx_k_ k;
struct s2_t0_tcp_rx_tcp_ack_d d;
    
%%
    .align
    .param          tcp_rx_rtt_start
    .param          tcp_ack_slow

#define c_est c6
#define c_win_upd c5

tcp_ack_start:
    tblwr.l         d.flag, k.to_s2_flag

    /*
     * fast path if
     *      state == ESTABLISHED &&
     *      !(flag & FLAG_SLOWPATH) &&
     *      ack_seq <= snd_nxt &&
     *      ack_seq > snd_una
     *      rcv_wnd has not changed
     */
    smeqb           c1, k.to_s2_flag, FLAG_SLOWPATH, FLAG_SLOWPATH
    scwlt           c2, k.s1_s2s_snd_nxt, k.s1_s2s_ack_seq
    scwle           c3, k.s1_s2s_ack_seq, d.snd_una
    sne             c4, d.cc_flags, 0
    seq             c_est, d.state, TCP_ESTABLISHED
    sne             c_win_upd, d.snd_wnd, k.to_s2_window
    setcf           c7, [c1 | c2 | c3 | c4 | c_win_upd | !c_est]
    j.c7            tcp_ack_slow
    nop

    tblwr           d.num_dup_acks, 0
tcp_ack_fast:
    tblor.l         d.flag, FLAG_SND_UNA_ADVANCED
    phvwri          p.common_phv_process_ack_flag, 1
    phvwrpair       p.to_s4_cc_ack_signal, TCP_CC_ACK, \
                        p.to_s4_cc_flags, d.cc_flags
tcp_update_wl_fast:
    tblwr           d.snd_wl1, k.s1_s2s_ack_seq
tcp_snd_una_update_fast:
    sub             r1, k.s1_s2s_ack_seq, d.snd_una
bytes_acked_stats_update_start:
    CAPRI_STATS_INC(bytes_acked, r1[31:0], d.bytes_acked, p.to_s7_bytes_acked)
bytes_acked_stats_update_end:
    phvwr           p.to_s4_bytes_acked, r3[31:0]
    tblwr           d.snd_una, k.s1_s2s_ack_seq

    tblor.l         d.flag, FLAG_WIN_UPDATE

    /*
     * tell txdma we have work to do
     */
    phvwrmi         p.common_phv_pending_txdma, TCP_PENDING_TXDMA_SND_UNA_UPDATE, \
                        TCP_PENDING_TXDMA_SND_UNA_UPDATE
    phvwr           p.common_phv_snd_una, d.snd_una
    
    /*
     * Launch next stage
     */
tcp_ack_done:
    tblwr           d.snd_wnd, k.to_s2_window
    phvwr           p.to_s4_snd_wnd, k.to_s2_window
    phvwrpair       p.rx2tx_extra_snd_wnd, k.to_s2_window, \
                        p.rx2tx_extra_snd_una, d.snd_una
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_rx_rtt_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_RTT_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop

