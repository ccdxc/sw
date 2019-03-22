/*
 *  TCP ACK processing (ack received from peer)
 *
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
    .param          tcp_rx_rtt_win_upd_start

tcp_ack_win_upd_start:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_rx_rtt_win_upd_start)
    nop.e
    nop

