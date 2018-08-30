
#include "../defines.h"

#define BIT(n)					(1 << n)

#define LG2_RX_DESC_SIZE        (4)
#define LG2_RX_CMPL_DESC_SIZE   (4)
#define LG2_RX_QSTATE_SIZE      (6)

#define RSS_TYPE_NONE         	(0)

#define RSS_TYPE_IPV4         	BIT(0)
#define RSS_TYPE_IPV4_TCP     	BIT(1)
#define RSS_TYPE_IPV4_UDP     	BIT(2)
#define RSS_TYPE_IPV6         	BIT(3)
#define RSS_TYPE_IPV6_TCP     	BIT(4)
#define RSS_TYPE_IPV6_UDP     	BIT(5)
#define RSS_TYPE_IPV6_EX      	BIT(6)
#define RSS_TYPE_IPV6_TCP_EX  	BIT(7)
#define RSS_TYPE_IPV6_UDP_EX  	BIT(8)

#define RSS_NONE              	(RSS_TYPE_NONE)
#define RSS_IPV4              	(RSS_TYPE_IPV4)
#define RSS_IPV4_TCP          	(RSS_TYPE_IPV4 | RSS_TYPE_IPV4_TCP)
#define RSS_IPV4_UDP          	(RSS_TYPE_IPV4 | RSS_TYPE_IPV4_UDP)
#define RSS_IPV6              	(RSS_TYPE_IPV6)
#define RSS_IPV6_TCP          	(RSS_TYPE_IPV6 | RSS_TYPE_IPV6_TCP)
#define RSS_IPV6_UDP          	(RSS_TYPE_IPV6 | RSS_TYPE_IPV6_UDP)

#define ETH_DMA_CMD_START_OFFSET    (CAPRI_PHV_START_OFFSET(dma_dma_cmd_type) / 16)
#define ETH_DMA_CMD_START_FLIT      ((offsetof(p, dma_dma_cmd_type) / 512) + 1)
#define ETH_DMA_CMD_START_INDEX     0

#define DMA_PKT(_r_ptr, _r_addr, _gs_len) \
    or          _r_addr, d.addr_lo, d.addr_hi, sizeof(d.addr_lo); \
    add         _r_addr, r0, _r_addr.dx; \
    or          _r_addr, _r_addr[63:16], _r_addr[11:8], sizeof(d.addr_lo); \
    DMA_PKT2MEM(_r_ptr, !c0, k.eth_rx_global_host_queue, _r_addr, _gs_len);

#define DEBUG_DESCR_FLD(name) \
    add         r7, r0, d.##name

#define DEBUG_DESCR(n) \
    DEBUG_DESCR_FLD(addr_lo##n); \
    DEBUG_DESCR_FLD(addr_hi##n); \
    DEBUG_DESCR_FLD(rsvd0##n); \
    DEBUG_DESCR_FLD(rsvd1##n); \
    DEBUG_DESCR_FLD(len##n); \
    DEBUG_DESCR_FLD(opcode##n); \
    DEBUG_DESCR_FLD(rsvd2##n); \
    DEBUG_DESCR_FLD(rsvd3##n); \
    DEBUG_DESCR_FLD(rsvd4##n)
