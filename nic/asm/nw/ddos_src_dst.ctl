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
    flow_lkp_metadata_lkp_src                       = 0x1;
    flow_lkp_metadata_lkp_dst                       = 0x1;
    flow_lkp_metadata_lkp_dport                     = 0x1;
    };

