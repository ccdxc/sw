#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct phv_ p;

%%

remove_tunnel_hdrs:
  phvwr.e     p.{vxlan_valid...gre_valid}, 0
  phvwr       p.{mpls_2_valid...mpls_0_valid}, 0
