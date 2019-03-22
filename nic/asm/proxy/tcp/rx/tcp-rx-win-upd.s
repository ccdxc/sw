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
    .align

tcp_rx_win_upd_process_start:
    phvwr           p.to_s5_serq_pidx, d.u.tcp_rx_d.serq_pidx
    phvwr           p.to_s5_rcv_nxt, d.u.tcp_rx_d.rcv_nxt
    phvwrmi         p.common_phv_pending_txdma, TCP_PENDING_TXDMA_ACK_SEND, \
                        TCP_PENDING_TXDMA_ACK_SEND
    phvwr           p.rx2tx_extra_rcv_nxt, d.u.tcp_rx_d.rcv_nxt
    phvwr           p.rx2tx_extra_state, d.u.tcp_rx_d.state
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_ack_win_upd_start)
    nop.e
    nop
