
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s7_t1_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s7_t1_k_ k;


#define _r_base                 r1  // Stats base address
#define _r_lif_offset           r2  // Offset of LIF's stats relative to global stats base
#define _r_offset               r3  // Offset of the cache group relative to global stats base
#define _r_addr                 r4  // Update address
#define _r_val                  r5  // Update value

%%

.param lif_stats_base

.align
eth_rx_stats_atomic:

#ifdef GFT
    b               eth_rx_stats_atomic_done
    nop
#endif

    addi            _r_base, r0, ASIC_MEM_SEM_ATOMIC_ADD_START
    addi            _r_lif_offset, r0, lif_stats_base[30:0] // substract 0x80000000 because hw adds it
    add             _r_lif_offset, _r_lif_offset, k.eth_rx_global_lif, LIF_STATS_SIZE_SHIFT

    bbeq            k.eth_rx_global_drop, 1, eth_rx_stats_atomic_incr_drop
    nop

eth_rx_stats_atomic_incr_accept:
    seq             c2, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_UNICAST
    addi.c2         _r_offset, _r_lif_offset, LIF_STATS_RX_UCAST_BYTES_OFFSET
    seq             c3, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_MULTICAST
    addi.c3         _r_offset, _r_lif_offset, LIF_STATS_RX_MCAST_BYTES_OFFSET
    seq             c4, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_BROADCAST
    addi.c4         _r_offset, _r_lif_offset, LIF_STATS_RX_BCAST_BYTES_OFFSET

    ATOMIC_INC_VAL_2(_r_base, _r_offset, _r_addr, _r_val, k.eth_rx_t1_s2s_pkt_len, 1)

    // Update operation counters
    addi            _r_offset, _r_lif_offset, LIF_STATS_RX_RSS_OFFSET
    ATOMIC_INC_VAL_6(_r_base, _r_offset, _r_addr, _r_val,
                    k.eth_rx_global_stats[STAT_oper_rss],
                    k.eth_rx_global_stats[STAT_oper_csum_complete],
                    k.eth_rx_global_stats[STAT_oper_csum_ip_bad],
                    k.eth_rx_global_stats[STAT_oper_csum_tcp_bad],
                    k.eth_rx_global_stats[STAT_oper_csum_udp_bad],
                    k.eth_rx_global_stats[STAT_oper_vlan_strip])

    b               eth_rx_stats_atomic_done
    nop

eth_rx_stats_atomic_incr_drop:
    seq             c2, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_UNICAST
    addi.c2         _r_offset, _r_lif_offset, LIF_STATS_RX_UCAST_DROP_BYTES_OFFSET
    seq             c3, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_MULTICAST
    addi.c3         _r_offset, _r_lif_offset, LIF_STATS_RX_MCAST_DROP_BYTES_OFFSET
    seq             c4, k.eth_rx_t1_s2s_l2_pkt_type, PACKET_TYPE_BROADCAST
    addi.c4         _r_offset, _r_lif_offset, LIF_STATS_RX_BCAST_DROP_BYTES_OFFSET

    ATOMIC_INC_VAL_2(_r_base, _r_offset, _r_addr, _r_val, k.eth_rx_t1_s2s_pkt_len, 1)

    addi            _r_offset, _r_lif_offset, LIF_STATS_RX_QUEUE_DISABLED_OFFSET
    ATOMIC_INC_VAL_5(_r_base, _r_offset, _r_addr, _r_val,
                    k.eth_rx_global_stats[STAT_queue_disabled],
                    k.eth_rx_global_stats[STAT_queue_empty],
                    k.eth_rx_global_stats[STAT_queue_error],
                    k.eth_rx_global_stats[STAT_desc_fetch_error],
                    k.eth_rx_global_stats[STAT_desc_data_error])

eth_rx_stats_atomic_done:
    // End of pipeline - Make sure no more tables will be launched
    phvwri.e.f      p.{app_header_table0_valid...app_header_table3_valid}, 0
    nop
