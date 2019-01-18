/*
 *  Implements the RTT stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s4_t0_tcp_rx_k.h"

struct phv_ p;
struct s4_t0_tcp_rx_k_ k;
struct s4_t0_tcp_rx_tcp_cc_d d;


%%
    .param          tcp_rx_fc_stage_start
    .param          tcp_cc_new_reno
    .align
tcp_rx_cc_stage_start:
    add r1, d.cc_algo, r0
    .brbegin
        br r1[0:0]
        nop
        .brcase TCP_CC_ALGO_NONE
            // Error
            b tcp_rx_cc_stage_end
            nop
        .brcase TCP_CC_ALGO_NEW_RENO
            j tcp_cc_new_reno
            nop
    .brend

tcp_rx_cc_stage_end:
    phvwr           p.rx2tx_extra_snd_cwnd, d.snd_cwnd
    CAPRI_NEXT_TABLE_READ_OFFSET_e(0, TABLE_LOCK_EN,
                        tcp_rx_fc_stage_start,
                        k.common_phv_qstate_addr,
                        TCP_TCB_FC_OFFSET, TABLE_SIZE_512_BITS)
    nop


