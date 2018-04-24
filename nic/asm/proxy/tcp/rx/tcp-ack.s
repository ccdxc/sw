/*
 *  TCP ACK processing (ack received from peer)
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"  
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_tcp_rx_k.h"
    
struct phv_ p;
struct s3_t0_tcp_rx_k_ k;
struct s3_t0_tcp_rx_tcp_ack_d d;
    
%%
    .align
    .param          tcp_rx_rtt_start
    .param          tcp_ack_slow

tcp_ack_start:
    tblwr.l         d.flag, k.to_s3_flag

    seq             c1, k.s1_s2s_ack_seq, d.snd_una
    b.c1            tcp_ack_done

    /*
     * fast path if
     *      state == ESTABLISHED &&
     *      !(flag & FLAG_SLOWPATH) &&
     *      ack_seq <= snd_nxt &&
     *      ack_seq > snd_una
     */
    sne             c1, d.state, TCP_ESTABLISHED
    smeqb           c2, k.to_s3_flag, FLAG_SLOWPATH, FLAG_SLOWPATH
    slt             c3, k.s1_s2s_snd_nxt, k.s1_s2s_ack_seq
    slt             c4, k.s1_s2s_ack_seq, d.snd_una
    setcf           c7, [c1 | c2 | c3 | c4]
    j.c7            tcp_ack_slow
    nop

tcp_ack_fast:
    tblor.l         d.flag, FLAG_SND_UNA_ADVANCED
tcp_update_wl_fast:
    tblwr           d.snd_wl1, k.s1_s2s_ack_seq
tcp_snd_una_update_fast:
    sub             r1, k.s1_s2s_ack_seq, d.snd_una
    tbladd          d.bytes_acked, r1
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
    phvwr           p.rx2tx_rx_flag, d.flag
    phvwr           p.rx2tx_snd_una, d.snd_una
    CAPRI_NEXT_TABLE_READ_OFFSET(0, TABLE_LOCK_EN,
                        tcp_rx_rtt_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_RTT_OFFSET, TABLE_SIZE_512_BITS)
    nop.e
    nop
