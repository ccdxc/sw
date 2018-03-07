/*
 *    Implements the RX stage of the RxDMA P4+ pipeline
 */

#include "tcp-constants.h"
#include "tcp-phv.h"
#include "tcp-shared-state.h"
#include "tcp-macros.h"
#include "tcp-table.h"
#include "tls_common.h"
#include "ingress.h"
#include "INGRESS_p.h"
#include "INGRESS_s6_t3_tcp_rx_k.h"

struct phv_ p;
struct s6_t3_tcp_rx_k_ k;
struct s6_t0_tcp_rx_write_serq_d d;

%%
    .param          tcp_rx_stats_stage_start
    .align

    /*
     * Global conditional variables
     * c7 drop packet and don't send to arm
     */
tcp_rx_write_serq_stage_start3:
    CAPRI_CLEAR_TABLE_VALID(0)
    CAPRI_CLEAR_TABLE_VALID(3)

    seq         c1, k.common_phv_write_arq, 1
    seq         c2, k.common_phv_write_serq, 1
    seq         c3, k.common_phv_l7_proxy_type_redirect, 1
    setcf       c7, [!c1 & !c2 & !c3]
    seq         c4, k.common_phv_skip_pkt_dma, 1
    setcf       c7, [c7 | c4]
    seq         c3, k.common_phv_fatal_error, 1
    bcf         [c7 | c3], tcp_write_serq3_done
    nop
    bcf         [c1], tcp_write_serq3_done
    nop

stats:
    smeqb       c1, k.common_phv_debug_dol, TCP_DDOL_TEST_ATOMIC_STATS, TCP_DDOL_TEST_ATOMIC_STATS
    bcf         [!c1], pkts_rcvd_stats_update_start
    nop
    // Debug: Force increment of atomic stats
debug_pkts_rcvd_stats_update_start:
    phvwr       p.to_s7_pkts_rcvd, 1
    phvwr       p.to_s7_pages_alloced, 1
    phvwr       p.to_s7_desc_alloced, 1
    phvwr       p.to_s7_debug_num_pkt_to_mem, 1
    // End debug stats increment, skip regular stats update
    b           tcp_write_serq_stats_end
    nop

    // Non-debug stats increment
pkts_rcvd_stats_update_start:
    seq         c1, k.common_phv_skip_pkt_dma, 1
    bcf         [c1], tcp_write_serq3_done

    CAPRI_STATS_INC(pkts_rcvd, 8, 1, d.pkts_rcvd)
pkts_rcvd_stats_update:
    CAPRI_STATS_INC_UPDATE(1, d.pkts_rcvd, p.to_s7_pkts_rcvd)
pkts_rcvd_stats_update_end:

    seq         c1, k.common_phv_ooo_in_rx_q, 1
    bcf         [c1], tcp_write_serq_stats_end
pages_alloced_stats_update_start:
    CAPRI_STATS_INC(pages_alloced, 8, 1, d.pages_alloced)
pages_alloced_stats_update:
    CAPRI_STATS_INC_UPDATE(1, d.pages_alloced, p.to_s7_pages_alloced)
pages_alloced_stats_update_end:

desc_alloced_stats_update_start:
    CAPRI_STATS_INC(desc_alloced, 8, 1, d.desc_alloced)
desc_alloced_stats_update:
    CAPRI_STATS_INC_UPDATE(1, d.desc_alloced, p.to_s7_desc_alloced)
desc_alloced_stats_update_end:

debug_num_pkt_to_mem_stats_update_start:
    CAPRI_STATS_INC(debug_num_pkt_to_mem, 8, 1, d.debug_num_pkt_to_mem)
debug_num_pkt_to_mem_stats_update:
    CAPRI_STATS_INC_UPDATE(1, d.debug_num_pkt_to_mem, p.to_s7_debug_num_pkt_to_mem)
debug_num_pkt_to_mem_stats_update_end:
tcp_write_serq_stats_end:

    CAPRI_NEXT_TABLE0_READ_NO_TABLE_LKUP(tcp_rx_stats_stage_start)

tcp_write_serq3_done:
    nop.e
    nop
    
