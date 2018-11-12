#include "ingress.h"
#include "INGRESS_p.h"

struct flow_lkp_k   k;
struct flow_lkp_d   d;
struct phv_         p;

%%

flow_info:
    phvwr           p.flow_metadata_entropy_hash, r1
    seq             c2, d.flow_info_d.entry_valid, 1
    bcf             [!c1|!c2], flow_miss
    nop
    phvwr           p.flow_metadata_snat_valid, d.flow_info_d.snat_valid
    phvwr           p.flow_metadata_tunnel_valid, d.flow_info_d.tunnel_valid
    phvwr.e         p.flow_metadata_snat_index, d.flow_info_d.snat_index
    phvwr           p.flow_metadata_tunnel_index, d.flow_info_d.tunnel_index

flow_miss:
    nop.e
    nop
