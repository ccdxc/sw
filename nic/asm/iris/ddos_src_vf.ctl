#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct ddos_src_vf_k   k;
struct ddos_src_vf_d   d;
struct phv_             p;

d = {
    u.ddos_src_vf_hit_d.ddos_src_vf_base_policer_idx = 0x1;
};

k = {
    control_metadata_src_lport = 0x1;
};

