#ifndef __HAL_PD_CPUPKT_HEADERS_HPP__
#define __HAL_PD_CPUPKT_HEADERS_HPP__

#include <base.h>
#include <sys/param.h>

namespace hal {
namespace pd {

typedef struct p4_to_p4plus_cpu_pkt_s {
#if __BYTE_ORDER == __BIG_ENDIAN
    uint64_t    pad             : 5;
    uint64_t    src_lif         : 11;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    uint64_t    src_lif         : 11;
    uint64_t    pad             : 5;
#else
#error "architecture unknown"
#endif

    uint64_t    reason          : 8;    

#if __BYTE_ORDER == __BIG_ENDIAN
    uint64_t    lkp_type        : 4;
    uint64_t    src_iport       : 4;
#else
    uint64_t    src_iport       : 4;
    uint64_t    lkp_type        : 4;
#endif

    uint64_t    lkp_vrf         : 16;
    uint64_t    flags           : 24;

    // outer
    uint64_t    mac_sa_outer    : 48;    
    uint64_t    mac_da_outer    : 48;

#if __BYTE_ORDER == __BIG_ENDIAN
    uint64_t    vlan_pcp_outer  : 3;
    uint64_t    vlan_dei_outer  : 1;
    uint64_t    vlan_id_outer   : 12;
#else
    uint64_t    vlan_id_outer   : 12;
    uint64_t    vlan_dei_outer  : 1;
    uint64_t    vlan_pcp_outer  : 3;
#endif

    uint8_t     ip_sa_outer[16];
    uint8_t     ip_da_outer[16];
    uint64_t    ip_proto_outer  : 8;
    uint64_t    ip_ttl_outer    : 8;
    uint64_t    l4_sport_outer  : 16;
    uint64_t    l4_dport_outer  : 16;

    // inner
    uint64_t    mac_sa_inner    : 48;    
    uint64_t    mac_da_inner    : 48;

#if __BYTE_ORDER == __BIG_ENDIAN
    uint64_t    vlan_pcp_inner  : 3;
    uint64_t    vlan_dei_inner  : 1;
    uint64_t    vlan_id_inner   : 28;
#else
    uint64_t    vlan_id_inner   : 28;
    uint64_t    vlan_dei_inner  : 1;
    uint64_t    vlan_pcp_inner  : 3;
#endif
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

typedef struct p4plus_to_p4_header_s {
    uint64_t    p4plus_app_id   : 4;
    uint64_t    pad             : 4;
    uint64_t    flags           : 8;
    uint64_t    ip_id           : 16;
    uint64_t    ip_len          : 16;
    uint64_t    udp_len         : 16;
    uint64_t    tcp_seq_delta   : 32;
    uint64_t    vlan_tag        : 16;
} __attribute__ ((__packed__)) p4plus_to_p4_header_t;
    
    
} // namespace pd       
} // namespace hal
#endif
