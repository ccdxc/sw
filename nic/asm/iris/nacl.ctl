#include "ingress.h"
#include "INGRESS_p.h"

struct nacl_k k;
struct nacl_d d;
struct phv_   p;

d = {
  u.nacl_permit_d.force_flow_hit = 1;
  u.nacl_permit_d.log_en = 1;
  u.nacl_permit_d.qid_en = 1;
  u.nacl_permit_d.qid = 0xbababa;
  u.nacl_permit_d.policer_index = 0xe;
  u.nacl_permit_d.ingress_mirror_session_id = 0x88;
};

#if 0
k = {
  control_metadata_flow_miss = 1;
};
#endif /* 0 */
