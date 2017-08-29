#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct l4_profile_k k;
struct l4_profile_d d;
struct phv_         p;

k = {
  inner_ipv4_srcAddr = 192.168.1.1;
  inner_ipv4_dstAddr = 192.168.1.2;
  flow_lkp_metadata_lkp_type = FLOW_KEY_LOOKUP_TYPE_IPV4;
  tunnel_metadata_tunnel_terminate = 1;
  flow_lkp_metadata_ipv4_flags = 2;
};

d = {
  u.l4_profile_d.ip_normalization_en = 1;
  u.l4_profile_d.ip_df_action = NORMALIZATION_ACTION_EDIT;
};
