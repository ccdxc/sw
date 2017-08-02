#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct mirror_k k;
struct mirror_d d;
struct phv_     p;

d = {
  u.erspan_mirror_d.dst_lif = 0x7ab;
  u.erspan_mirror_d.truncate_len = 64;
  u.erspan_mirror_d.tunnel_rewrite_index = 0xca;
};
