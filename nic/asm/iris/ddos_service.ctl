#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct ddos_service_k   k;
struct ddos_service_d   d;
struct phv_             p;

d = {
    u.ddos_service_hit_d.ddos_service_base_policer_idx = 0x1;
    };

k = {
    flow_lkp_metadata_lkp_vrf               = 0x1;
    flow_lkp_metadata_lkp_proto             = 0x1;
    tcp_flags                               = 0x2;
    flow_lkp_metadata_lkp_dst               = 0x1;
    flow_lkp_metadata_lkp_dport             = 0x1;
    };

