/*
 *	Implements the rx2tx shared extra state read stage of the TxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s1_t0_tcp_tx_k.h"
	
struct phv_ p;
struct s1_t0_tcp_tx_k_ k;
struct s1_t0_tcp_tx_read_rx2tx_extra_d d;
	
%%
    .align
    .param      tcp_tx_process_pending_start
    .param      tcp_tx_s2_bubble_start

tcp_tx_read_rx2tx_shared_extra_clean_retx_stage1_start:
    /*
     * For snd_una_update, the next stage is launched by read-sesq-retx-ci
     * stage, so skip launching the next stage here. Also if we are dropping
     * the PHV, then set global_drop bit
     */
    phvwrpair       p.t0_s2s_snd_wnd, d.snd_wnd, \
                        p.t0_s2s_state, d.state

    // In close wait state, don't batch freeing of descriptors
    seq             c1, d.state, TCP_ESTABLISHED
    phvwr.!c1       p.t0_s2s_clean_retx_num_retx_pkts, 1

    phvwr.e         p.common_phv_snd_una, d.snd_una
    phvwr           p.t0_s2s_rcv_nxt, d.rcv_nxt
    nop
