/*
 *	Implements the stats stage of the TCP TxDMA P4+ pipeline
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"	
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t0_tcp_tx_k.h"
	
struct phv_ p;
struct s6_t0_tcp_tx_k_ k;
	
%%
    .param          TCP_PROXY_STATS
	.align	
tcp_tx_stats_start:
        CAPRI_CLEAR_TABLE_VALID(0)

// TODO: Move to multi stats update

// **NOTE: Offsets need to match definition in __tcp_tx_stats_t in 
// p4pd_tcp_proxy_api.h
bytes_sent_atomic_stats_update_start:
        CAPRI_ATOMIC_STATS_INCR1(bytes_sent, k.t0_s6_s2s_tx_stats_base,
                0 * 8, k.to_s6_bytes_sent)
bytes_sent_atomic_stats_update_done:

pkts_sent_atomic_stats_update_start:
        CAPRI_ATOMIC_STATS_INCR1(pkts_sent, k.t0_s6_s2s_tx_stats_base,
                1 * 8, k.to_s6_pkts_sent)
pkts_sent_atomic_stats_update_done:

pure_acks_sent_atomic_stats_update_start:
        CAPRI_ATOMIC_STATS_INCR1(pure_acks_sent, k.t0_s6_s2s_tx_stats_base,
                2 * 8, k.to_s6_pure_acks_sent)
pure_acks_sent_atomic_stats_update_done:

    seq             c1, k.common_phv_fin, 1
    balcf           r7, [c1], tcp_tx_update_fin_stats
    nop
    
    seq             c1, k.common_phv_rst, 1
    balcf           r7, [c1], tcp_tx_update_rst_stats
    nop

tcp_rx_stats_done:
	nop.e
	nop

tcp_tx_update_fin_stats:
    addui           r2, r0, hiword(TCP_PROXY_STATS)
    addi            r2, r2, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r2, TCP_PROXY_STATS_FIN_SENT, 1)
    jr              r7
    nop 

tcp_tx_update_rst_stats:
    addui           r2, r0, hiword(TCP_PROXY_STATS)
    addi            r2, r2, loword(TCP_PROXY_STATS)
    CAPRI_ATOMIC_STATS_INCR1_NO_CHECK(r2, TCP_PROXY_STATS_RST_SENT, 1)
    jr              r7
    nop 
