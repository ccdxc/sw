#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ipsg_k k;
struct ipsg_d d;
struct phv_   p;

d = {
    u.ipsg_hit_d.src_lport  = 0x1;
    u.ipsg_hit_d.mac        = 0x1;
    u.ipsg_hit_d.vlan_valid = 0x1;
    u.ipsg_hit_d.vlan_id    = 0x1;
};

k = {
    vlan_tag_vid_sbit0_ebit3        = 0x0;
    vlan_tag_vid_sbit4_ebit11       = 0x1;
    vlan_tag_valid                  = 0x1;
    control_metadata_src_lport      = 0x1;
    ethernet_srcAddr                = 0x1;
    flow_lkp_metadata_lkp_type      = 0x1;
    flow_lkp_metadata_lkp_vrf       = 0x1;
    flow_lkp_metadata_lkp_src       = 0x1;
};
