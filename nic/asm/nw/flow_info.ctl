#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct flow_info_k k;
struct flow_info_d d;
struct phv_        p;

d = {
  u.flow_info_d.multicast_en = 0;
  u.flow_info_d.lif = 0xfa;
  u.flow_info_d.qid_en = 0;
  u.flow_info_d.ingress_mirror_session_id = 0x08;
  u.flow_info_d.ingress_policer_index = 0x64;
  u.flow_info_d.egress_policer_index = 0x65;
  u.flow_info_d.session_state_index = 0xface;
  u.flow_info_d.rewrite_index = 0x100;
};

r6 = 0x01100110;
