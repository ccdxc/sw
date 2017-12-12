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
struct s6_t0_tcp_rx_k k;
	
%%
	.align	
tcp_rx_stats_stage_start:
    CAPRI_CLEAR_TABLE_VALID(0)

// TODO: Move to multi stats update

// **NOTE: Offsets need to match definition in __tcp_rx_stats_t in 
// p4pd_tcp_proxy_api.h
bytes_rcvd_atomic_stats_update_start:
        CAPRI_ATOMIC_STATS_INCR1(bytes_rcvd, k.common_phv_qstate_addr,
                TCP_TCB_RX_STATS_OFFSET + 0 * 8, k.to_s6_bytes_rcvd)
bytes_rcvd_atomic_stats_update_done:

pkts_rcvd_atomic_stats_update_start:
        CAPRI_ATOMIC_STATS_INCR1(pkts_rcvd, k.common_phv_qstate_addr,
                TCP_TCB_RX_STATS_OFFSET + 1 * 8, k.to_s6_pkts_rcvd)
pkts_rcvd_atomic_stats_update_done:

pages_alloced_atomic_stats_update_start:
        CAPRI_ATOMIC_STATS_INCR1(pages_alloced, k.common_phv_qstate_addr,
                TCP_TCB_RX_STATS_OFFSET + 2 * 8, k.to_s6_pages_alloced)
pages_alloced_atomic_stats_update_done:

desc_alloced_atomic_stats_update_start:
        CAPRI_ATOMIC_STATS_INCR1(desc_alloced, k.common_phv_qstate_addr,
                TCP_TCB_RX_STATS_OFFSET + 3 * 8, k.to_s6_desc_alloced)
desc_alloced_atomic_stats_update_done:

debug_num_phv_to_mem_atomic_stats_update_start:
        CAPRI_ATOMIC_STATS_INCR1(debug_num_phv_to_mem, k.common_phv_qstate_addr,
                TCP_TCB_RX_STATS_OFFSET + 4 * 8, k.to_s6_debug_num_phv_to_mem)
debug_num_phv_to_mem_atomic_stats_update_done:

debug_num_pkt_to_mem_atomic_stats_update_start:
        CAPRI_ATOMIC_STATS_INCR1(debug_num_pkt_to_mem, k.common_phv_qstate_addr,
                TCP_TCB_RX_STATS_OFFSET + 5 * 8, k.to_s6_debug_num_pkt_to_mem)
debug_num_pkt_to_mem_atomic_stats_update_done:

tcp_rx_stats_stage_done:
	nop.e
	nop

