#ifndef ___HAL_PD_CPU_PKT_API_HPP__
#define ___HAL_PD_CPU_PKT_API_HPP__

#include <base.h>
#include <pd.hpp>

namespace hal {
namespace pd {

#define MAX_CPU_PKT_QUEUES   4
typedef uint64_t  cpupkt_hw_id_t;

typedef struct cpupkt_queue_info_s {
    types::WRingType    type;
    cpupkt_hw_id_t      base_addr;
    uint32_t            cindex;
    cpupkt_hw_id_t      cindex_addr;
    pd_wring_meta_t*    wring_meta; 
} cpupkt_queue_info_t;

typedef struct cpupkt_rx_ctxt_s {
    uint32_t                num_queues;
    cpupkt_queue_info_t     queue[MAX_CPU_PKT_QUEUES];
} __PACK__ cpupkt_rx_ctxt_t;

typedef struct cpupkt_tx_ctxt_s {
    uint32_t    send_queue;
    uint32_t    pindex;   
} __PACK__ cpupkt_tx_ctxt_t;

static inline cpupkt_rx_ctxt_t *
cpupkt_rx_ctxt_init(cpupkt_rx_ctxt_t* ctxt)
{
    if(!ctxt) {
        return NULL;    
    }
    
    ctxt->num_queues = 0;
    return ctxt;
}

typedef struct p4_to_p4plus_cpu_pkt_s {
    uint64_t    pad             : 5;
    uint64_t    src_lif         : 11;
    uint64_t    reason          : 8;    

    uint64_t    lkp_type        : 4;
    uint64_t    src_iport       : 4;
    uint64_t    lkp_vrf         : 16;
    uint64_t    flags           : 24;

    // outer
    uint64_t    mac_sa_outer    : 48;    
    uint64_t    mac_da_outer    : 48;    
    uint64_t    vlan_pcp_outer  : 3;
    uint64_t    vlan_dei_outer  : 1;
    uint64_t    vlan_id_outer   : 12;
    uint8_t     ip_sa_outer[16];
    uint8_t     ip_da_outer[16];
    uint64_t    ip_proto_outer  : 8;
    uint64_t    ip_ttl_outer    : 8;
    uint64_t    l4_sport_outer  : 16;
    uint64_t    l4_dport_outer  : 16;

    // inner
    uint64_t    mac_sa_inner    : 48;    
    uint64_t    mac_da_inner    : 48;    
    uint64_t    vlan_pcp_inner  : 3;
    uint64_t    vlan_dei_inner  : 1;
    uint64_t    vlan_id_inner   : 28;
    uint8_t     ip_sa_inner[16];
    uint8_t     ip_da_inner[16];
    uint64_t    ip_proto_inner  : 8;
    uint64_t    ip_ttl_inner    : 8;
    uint64_t    l4_sport_inner  : 16;
    uint64_t    l4_dport_inner  : 16;

    // tcp
    uint64_t    tcp_flags       : 8;
    uint64_t    tcp_seqNo       : 32;
    uint64_t    tcp_AckNo       : 32;
    uint64_t    tcp_window      : 16;
    uint64_t    tcp_options     : 8;
    uint64_t    tcp_mss         : 16;
    uint64_t    tcp_ws          : 8;
} __attribute__ ((__packed__)) p4_to_p4plus_cpu_pkt_t;

cpupkt_rx_ctxt_t* cpupkt_rx_ctxt_alloc_init(void);
hal_ret_t cpupkt_register_rx_queue(cpupkt_rx_ctxt_t* ctxt, types::WRingType type);
hal_ret_t cpupkt_poll_receive(cpupkt_rx_ctxt_t* ctxt,
                              p4_to_p4plus_cpu_pkt_t** flow_miss_hdr,
                              uint8_t** data, 
                              size_t* data_len);
hal_ret_t cpupkt_free(p4_to_p4plus_cpu_pkt_t* flow_miss_hdr, uint8_t* data);

} // namespace pd
} // namespace hal

#endif
