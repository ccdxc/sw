#pragma once

#include <sys/param.h>
#include "nic/include/pkt_hdrs.hpp"

namespace hal {
namespace pd {

typedef struct p4_to_p4plus_cpu_pkt_s {
    uint16_t   src_lif;

    uint16_t  lif;
    uint8_t   qtype;
    uint32_t  qid;

    uint16_t  lkp_vrf;

#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t   pad      : 2;
    uint8_t   lkp_dir  : 1;
    uint8_t   lkp_inst : 1;
    uint8_t   lkp_type : 4;
#elif __BYTE_ORDER == __LITTLE_ENDIAN
    uint8_t   lkp_type : 4;
    uint8_t   lkp_inst : 1;
    uint8_t   lkp_dir  : 1;
    uint8_t   pad      : 2;
#else
#error "architecture unknown"
#endif

    uint8_t   flags;

    // offsets
    int16_t  l2_offset;
    int16_t  l3_offset;
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

    // tcp
    uint8_t   tcp_flags;
    uint32_t  tcp_seq_num;
    uint32_t  tcp_ack_num;
    uint16_t  tcp_window;
    uint8_t   tcp_options;
    uint16_t  tcp_mss;
    uint8_t   tcp_ws;

} __attribute__ ((__packed__)) p4_to_p4plus_cpu_pkt_t;

typedef struct p4plus_to_p4_header_s {
#if __BYTE_ORDER == __BIG_ENDIAN
    uint8_t     p4plus_app_id   : 4;
    uint8_t     pad             : 4;
#else
    uint8_t     pad             : 4;
    uint8_t     p4plus_app_id   : 4;
#endif

    uint8_t     flags;
    uint8_t     udp_opt_bytes;
    uint8_t     rsvd[3];
    uint16_t    ip_id_delta;
    uint32_t    tcp_seq_delta;
    uint32_t    gso_start:14;
    uint32_t    byte_align_pad0:2;
    uint32_t    gso_offset:14;
    uint32_t    byte_align_pad1:1;
    uint32_t    gso_valid:1;
    uint16_t    vlan_tag;
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
