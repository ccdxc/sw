#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct flow_info_k k;
struct flow_info_d d;
struct phv_        p;

d = {
  u.flow_info_d.multicast_en = 0;
  u.flow_info_d.dst_lport = 0xfa;
  u.flow_info_d.qid_en = 0;
  u.flow_info_d.ingress_mirror_session_id = 0x08;
  u.flow_info_d.session_state_index = 0xface;
  u.flow_info_d.rewrite_index = 0x100;
  u.flow_info_d.expected_src_lif_check_en = 0;
  u.flow_info_d.expected_src_lif = 0x01ba;
};

k = {
  control_metadata_src_lif = 0x01bb;
  p4plus_to_p4_p4plus_app_id = 0;
};

r6 = 0x01100110;
