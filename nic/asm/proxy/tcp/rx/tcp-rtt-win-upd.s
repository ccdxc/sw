/*
 *  Implements the RTT stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s3_t0_tcp_rx_k.h"
    
struct phv_ p;
struct s3_t0_tcp_rx_k_ k;
struct s3_t0_tcp_rx_tcp_rtt_d d;

    
%%
    .param          tcp_rx_cc_win_upd_stage_start
    .align
tcp_rx_rtt_win_upd_start:
    CAPRI_NEXT_TABLE_READ_NO_TABLE_LKUP(0, tcp_rx_cc_win_upd_stage_start)
    nop.e
    nop
