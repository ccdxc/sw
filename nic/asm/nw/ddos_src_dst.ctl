#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ddos_src_dst_k   k;
struct ddos_src_dst_d   d;
struct phv_             p;

d = {
    u.ddos_src_dst_hit_d.ddos_src_dst_base_policer_idx = 0x1;
};

k = {
    flow_lkp_metadata_lkp_vrf                       = 0x1;
    flow_lkp_metadata_lkp_proto                     = 0x1;
    tcp_flags                                       = 0x2;
    //flow_lkp_metadata_lkp_src                       = 0x1;
    flow_lkp_metadata_lkp_dport                     = 0x1;

    // ICMP Normalization Fields
    control_metadata_flow_miss_ingress              = 0x0;
    icmp_valid                                      = 0x1;
    l4_metadata_icmp_normalization_en               = 0x1;
    icmp_typeCode                                   = 0x1001; // icmp_type = 8;
    l4_metadata_icmp_deprecated_msgs_drop           = 0x1;
    l4_metadata_icmp_redirect_msg_drop              = 0x1;
    l4_metadata_icmp_invalid_code_action            = 0x3;
};

p = {

    icmp_typeCode                                   = 0x0801;
};

