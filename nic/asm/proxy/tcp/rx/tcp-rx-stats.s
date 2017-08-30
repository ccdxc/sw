/*
 *	Implements the stats stage of the TCP RxDMA P4+ pipeline
 */


#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tcp-constants.h"	
#include "ingress.h"
#include "INGRESS_p.h"
	
struct phv_ p;
struct tcp_rx_stats_k k;
	
%%
	.align	
tcp_rx_stats_stage7_start:
        seq             c1, k.to_s7_bytes_rcvd, 0
        bcf             [c1], tcp_rx_stats_stage7_done
        CAPRI_ATOMIC_STATS_INCR1(k.common_phv_qstate_addr, (64 * 11), k.to_s7_bytes_rcvd)
tcp_rx_stats_stage7_done:
	nop.e
	nop

