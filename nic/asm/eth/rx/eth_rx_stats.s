
#include "INGRESS_p.h"
#include "ingress.h"
#include "INGRESS_rx_table_s4_t3_k.h"

#include "defines.h"

struct phv_ p;
struct rx_table_s4_t3_k_ k;

#define _r_base                 r1  // Stats base address
#define _r_lif_offset           r2  // Offset of LIF's stats relative to global stats base
#define _r_offset               r3  // Offset of the cache group relative to global stats base
#define _r_addr                 r4  // Update address
#define _r_val                  r5  // Update value

%%

.align
eth_rx_stats:
    addi            _r_base, r0, CAPRI_MEM_SEM_ATOMIC_ADD_START
    add             _r_lif_offset, r0, k.eth_rx_global_lif, LIF_STATS_SIZE_SHIFT

    // Update packet counters
    addi            _r_offset, _r_lif_offset, LIF_STATS_RX_UCAST_BYTES_OFFSET
    ATOMIC_INC_VAL_6(_r_base, _r_offset, _r_addr, _r_val,
                    k.eth_rx_global_stats[STAT_unicast_bytes],
                    k.eth_rx_global_stats[STAT_unicast_packets],
                    k.eth_rx_global_stats[STAT_multicast_bytes],
                    k.eth_rx_global_stats[STAT_multicast_packets],
                    k.eth_rx_global_stats[STAT_broadcast_bytes],
                    k.eth_rx_global_stats[STAT_broadcast_packets])

    // Update packet drop counters
    addi            _r_offset, _r_lif_offset, LIF_STATS_RX_UCAST_DROP_BYTES_OFFSET
    ATOMIC_INC_VAL_6(_r_base, _r_offset, _r_addr, _r_val,
                    k.eth_rx_global_stats[STAT_unicast_drop_bytes],
                    k.eth_rx_global_stats[STAT_unicast_drop_packets],
                    k.eth_rx_global_stats[STAT_multicast_drop_bytes],
                    k.eth_rx_global_stats[STAT_multicast_drop_packets],
                    k.eth_rx_global_stats[STAT_broadcast_drop_bytes],
                    k.eth_rx_global_stats[STAT_broadcast_drop_packets])

    // Update queue & desc counters
    addi            _r_offset, _r_lif_offset, LIF_STATS_RX_QUEUE_DISABLED_DROP_OFFSET
    ATOMIC_INC_VAL_5(_r_base, _r_offset, _r_addr, _r_val,
                    k.eth_rx_global_stats[STAT_queue_disabled_drop],
                    k.eth_rx_global_stats[STAT_queue_empty_drop],
                    k.eth_rx_global_stats[STAT_queue_scheduled],
                    k.eth_rx_global_stats[STAT_desc_fetch_error],
                    k.eth_rx_global_stats[STAT_desc_data_error])

    // Update operation counters
    addi            _r_offset, _r_lif_offset, LIF_STATS_RX_RSS_OFFSET
    ATOMIC_INC_VAL_6(_r_base, _r_offset, _r_addr, _r_val,
                    k.eth_rx_global_stats[STAT_oper_rss],
                    k.eth_rx_global_stats[STAT_oper_csum_complete],
                    k.eth_rx_global_stats[STAT_oper_csum_ip_bad],
                    k.eth_rx_global_stats[STAT_oper_csum_tcp_bad],
                    k.eth_rx_global_stats[STAT_oper_csum_udp_bad],
                    k.eth_rx_global_stats[STAT_oper_vlan_strip])

    nop.e
    nop
