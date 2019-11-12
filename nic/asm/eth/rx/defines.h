
#include "nic/asm/eth/defines.h"

#define LG2_RX_QSTATE_SIZE          (6)

#define LG2_RX_SG_MAX_READ_SIZE     (6)

#define LG2_RX_STATS_BLOCK_SZ       (6)

#define L2_PKT_TYPE_UNICAST     0
#define L2_PKT_TYPE_MULTICAST   1
#define L2_PKT_TYPE_BROADCAST   2

#define PKT_TYPE_NON_IP         (0)
#define PKT_TYPE_IPV4           BIT(0)
#define PKT_TYPE_IPV4_TCP       BIT(1)
#define PKT_TYPE_IPV4_UDP       BIT(2)
#define PKT_TYPE_IPV6           BIT(3)
#define PKT_TYPE_IPV6_TCP       BIT(4)
#define PKT_TYPE_IPV6_UDP       BIT(5)

#define RSS_NONE                (PKT_TYPE_NON_IP)   // 0x0
#define RSS_IPV4                (PKT_TYPE_IPV4)     // 0x1
#define RSS_IPV4_TCP            (PKT_TYPE_IPV4 | PKT_TYPE_IPV4_TCP)     // 0x3
#define RSS_IPV4_UDP            (PKT_TYPE_IPV4 | PKT_TYPE_IPV4_UDP)     // 0x5
#define RSS_IPV6                (PKT_TYPE_IPV6)     // 0x8
#define RSS_IPV6_TCP            (PKT_TYPE_IPV6 | PKT_TYPE_IPV6_TCP)     // 0x18
#define RSS_IPV6_UDP            (PKT_TYPE_IPV6 | PKT_TYPE_IPV6_UDP)     // 0x28

#define ETH_DMA_CMD_START_OFFSET    (CAPRI_PHV_START_OFFSET(dma_dma_cmd_type) / 16)
#define ETH_DMA_CMD_START_FLIT      ((offsetof(p, dma_dma_cmd_type) / 512) + 1)
#define ETH_DMA_CMD_START_INDEX     0

#define ETH_RX_DESC_ADDR_ERROR      1
#define ETH_RX_DESC_DATA_ERROR      2

/*
 * Stats Macros
 */

#define INIT_STATS(_r) \
    add         _r, r0, r0

#define LOAD_STATS(_r) \
    add         _r, r0, k.eth_rx_global_stats;

#define SET_STAT(_r, _c, name) \
    ori._c      _r, _r, (1 << STAT_##name);

#define CLR_STAT(_r, _c, name) \
    andi._c     _r, _r, ~(1 << STAT_##name);

#define SAVE_STATS(_r) \
    phvwr       p.eth_rx_global_stats, _r;

/*
 * Stat position within stats register
 */

// packet counters
#define STAT_unicast_bytes                  0
#define STAT_unicast_packets                1
#define STAT_multicast_bytes                2
#define STAT_multicast_packets              3
#define STAT_broadcast_bytes                4
#define STAT_broadcast_packets              5
// drop counters
#define STAT_unicast_drop_bytes             8
#define STAT_unicast_drop_packets           9
#define STAT_multicast_drop_bytes           10
#define STAT_multicast_drop_packets         11
#define STAT_broadcast_drop_bytes           12
#define STAT_broadcast_drop_packets         13
// queue & descriptor counters
#define STAT_queue_disabled                 16
#define STAT_queue_empty                    17
#define STAT_queue_error                    18
#define STAT_desc_fetch_error               19
#define STAT_desc_data_error                20
#define STAT_event_disabled                 21
#define STAT_event_error                    22
// DEBUG: operation counters
#define STAT_oper_rss                       24
#define STAT_oper_csum_complete             25
#define STAT_oper_csum_ip_bad               26
#define STAT_oper_csum_tcp_bad              27
#define STAT_oper_csum_udp_bad              28
#define STAT_oper_vlan_strip                29
// DEBUG: completion, event, and intr counters (XXX not counted)
#define STAT_arm                            31
#define STAT_cqe                            31
#define STAT_eqe                            31
#define STAT_intr                           31

/*
 * DMA Macros
 */

#define DMA_PKT(_r_ptr, _r_addr, _r_len) \
    add         _r_addr, r0, d.{addr}.dx; \
    DMA_PKT2MEM(_r_ptr, _C_FALSE, k.eth_rx_global_host_queue, _r_addr, _r_len, r7);

#define DMA_FRAG(n, _r_addr, _r_len, _r_ptr) \
    add         _r_addr, r0, d.{addr##n}.dx; \
    DMA_PKT2MEM(_r_ptr, _C_FALSE, k.eth_rx_global_host_queue, _r_addr, _r_len, r7);
