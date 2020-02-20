#pragma once

#include <sys/param.h>
#include "nic/include/pkt_hdrs.hpp"

#define L2HDR_DOT1Q_OFFSET              12

namespace hal {
namespace pd {

typedef struct p4_to_p4plus_cpu_pkt_s {
    uint16_t   src_lif;

    uint16_t  lif;
    uint8_t   qtype;
    uint32_t  qid;

    uint16_t  lkp_vrf;
    uint16_t  src_lport;

#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t   src_app_id : 4;
    uint8_t   lkp_dir    : 1;
    uint8_t   lkp_inst   : 1;
    uint8_t   lkp_type   : 2;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t   lkp_type   : 2;
    uint8_t   lkp_inst   : 1;
    uint8_t   lkp_dir    : 1;
    uint8_t   src_app_id : 4;
#else
#error "architecture unknown"
#endif

    uint8_t   flags;

    // offsets
    uint16_t  l2_offset;
    uint16_t  l3_offset;
    int16_t  l4_offset;
    int16_t  payload_offset;

    // qos
#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t   src_tm_iq : 5;
    uint8_t   pad_1     : 3;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t   pad_1     : 3;
    uint8_t   src_tm_iq : 5;
#else
#error "architecture unknown"
#endif

    // flow hash
    uint32_t  flow_hash;
    // tcp
    uint8_t   tcp_flags;
    uint32_t  tcp_seq_num;
    uint32_t  tcp_ack_num;
    uint16_t  tcp_window;
    uint8_t   tcp_options;
    uint16_t  tcp_mss;
    uint8_t   tcp_ws;

} __attribute__ ((__packed__)) p4_to_p4plus_cpu_pkt_t;

#define GET_IPV4_HEADER(pkt, cpu_hdr) (ipv4_header_t *)(pkt + cpu_hdr->l3_offset);
#define GET_IPV6_HEADER(pkt, cpu_hdr) (ipv6_header_t *)(pkt + cpu_hdr->l3_offset);
#define GET_L2_HEADER(pkt, cpu_hdr)   (ether_header_t *)(pkt + cpu_hdr->l2_offset);
#define GET_VLAN_HEADER(pkt, cpu_hdr) (vlan_header_t *)(pkt + cpu_hdr->l2_offset);
#define GET_ICMP_HEADER(pkt, cpu_hdr) (icmp_header_t *)(pkt + cpu_hdr->l4_offset);


typedef struct p4plus_to_p4_header_s {
#if __BYTE_ORDER == __BIG_ENDIAN
    uint32_t    p4plus_app_id           : 4;
    uint32_t    pad                     : 4;
    uint32_t    flow_index              : 24;
    uint32_t    lkp_inst                : 1;
    uint32_t    compute_inner_l4_csum   : 1;
    uint32_t    compute_l4_csum         : 1;
    uint32_t    update_udp_len          : 1;
    uint32_t    insert_vlan_tag         : 1;
    uint32_t    update_tcp_seq_no       : 1;
    uint32_t    update_ip_len           : 1;
    uint32_t    update_ip_id            : 1;
    uint32_t    udp_opt_bytes           : 8;
    uint32_t    dst_lport               : 11;
    uint32_t    dst_lport_valid         : 1;
    uint32_t    pad1                    : 4;
    uint32_t    ip_id_delta             : 16;
    uint32_t    tcp_seq_delta           : 32;
    uint32_t    gso_start               : 14;
    uint32_t    compute_inner_ip_csum   : 1;
    uint32_t    compute_ip_csum         : 1;
    uint32_t    gso_offset              : 14;
    uint32_t    flow_index_valid        : 1;
    uint32_t    gso_valid               : 1;
    uint32_t    vlan_tag                : 16;
#else
    uint32_t    vlan_tag                : 16;
    uint32_t    gso_valid               : 1;
    uint32_t    flow_index_valid        : 1;
    uint32_t    gso_offset              : 14;
    uint32_t    compute_ip_csum         : 1;
    uint32_t    compute_inner_ip_csum   : 1;
    uint32_t    gso_start               : 14;
    uint32_t    tcp_seq_delta           : 32;
    uint32_t    ip_id_delta             : 16;
    uint32_t    pad1                    : 4;
    uint32_t    dst_lport_valid         : 1;
    uint32_t    dst_lport               : 11;
    uint32_t    udp_opt_bytes           : 8;
    uint32_t    update_ip_id            : 1;
    uint32_t    update_ip_len           : 1;
    uint32_t    update_tcp_seq_no       : 1;
    uint32_t    update_udp_len          : 1;
    uint32_t    insert_vlan_tag         : 1;
    uint32_t    compute_l4_csum         : 1;
    uint32_t    compute_inner_l4_csum   : 1;
    uint32_t    lkp_inst                : 1;
    uint32_t    flow_index              : 24;
    uint32_t    pad                     : 4;
    uint32_t    p4plus_app_id           : 4;
#endif
} __attribute__ ((__packed__)) p4plus_to_p4_header_t;

typedef struct cpu_to_p4plus_header_s {
    uint16_t    flags;
    uint16_t    src_lif;
    uint16_t    hw_vlan_id;
    uint16_t    l2_offset;
    uint8_t     pad    : 3;
    uint8_t     tm_oq  : 5;
} __attribute__ ((__packed__)) cpu_to_p4plus_header_t;

} // namespace pd
} // namespace hal

struct quiesce_trailer_t {
    uint32_t timestamp;
} __attribute__ ((__packed__));
