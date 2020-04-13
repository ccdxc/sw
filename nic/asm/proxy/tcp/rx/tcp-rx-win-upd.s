/*
 *  Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_tcp_rx_k.h"

struct phv_ p;
struct s1_t0_tcp_rx_k_ k;
struct s1_t0_tcp_rx_d d;

%%
    .param          tcp_ack_win_upd_start
    .param          tcp_ooo_book_keeping_in_order
    .param          tcp_rx_read_rnmdr_fc
    .align

#define c_launch_ooq c7
#define c_dont_send_ack c_launch_ooq

tcp_rx_win_upd_process_start:

    // Read rnmdr unconditionally here. Need more accurate calculation of
    // the window when we send out window updates.
    CAPRI_NEXT_TABLE_READ_i(3, TABLE_LOCK_DIS, tcp_rx_read_rnmdr_fc,
                 ASIC_SEM_RNMDPR_BIG_ALLOC_RAW_ADDR, TABLE_SIZE_64_BITS)
    seq             c1, k.common_phv_ooq_tx2rx_win_upd, 1
    phvwr.c1        p.rx2tx_extra_pending_dup_ack_send, 1
    phvwr.c1        p.rx2tx_extra_dup_rcv_nxt, d.u.tcp_rx_d.rcv_nxt
    seq             c1, d.u.tcp_rx_d.ooq_not_empty, 1
    seq             c2, k.common_phv_ooq_tx2rx_last_ooo_pkt, 1

    // Launch OOQ table if ooq is not empty and this is the last tx2rx packet
    // Also don't send ack in this case
    setcf           c_launch_ooq, [c1 & c2]

    phvwr           p.to_s5_serq_pidx, d.u.tcp_rx_d.serq_pidx
    phvwr           p.to_s5_rcv_nxt, d.u.tcp_rx_d.rcv_nxt
    phvwrmi.!c_dont_send_ack \
                    p.common_phv_pending_txdma, TCP_PENDING_TXDMA_ACK_SEND, \
                        TCP_PENDING_TXDMA_ACK_SEND
    phvwr           p.rx2tx_extra_rcv_nxt, d.u.tcp_rx_d.rcv_nxt
    phvwr           p.rx2tx_extra_state, d.u.tcp_rx_d.state
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_ack_win_upd_start)

    // Launch OOO table only for last ooq_tx2rx_last_ooo_pkt
    b.!c_launch_ooq tcp_rx_win_upd_skip_ooo_launch
    nop

    // launch OOO
    phvwr           p.t2_s2s_seq, k.s1_s2s_seq
    phvwr           p.t2_s2s_payload_len, k.s1_s2s_payload_len
    // We don't want to advance ack_seq no until ooq packets are all dequeued
    phvwr           p.rx2tx_extra_pending_dup_ack_send, 1
    phvwr           p.rx2tx_extra_dup_rcv_nxt, d.u.tcp_rx_d.rcv_nxt
    CAPRI_NEXT_TABLE_READ_OFFSET(2, TABLE_LOCK_EN, tcp_ooo_book_keeping_in_order,
                        k.common_phv_qstate_addr, TCP_TCB_OOO_BOOK_KEEPING_OFFSET0,
                        TABLE_SIZE_512_BITS)

tcp_rx_win_upd_skip_ooo_launch:
    nop.e
    nop
