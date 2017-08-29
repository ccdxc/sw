#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct flow_hash_d d;
struct phv_        p;

d = {
  flow_hash_info_d.entry_valid = 1;
  flow_hash_info_d.flow_index = 0x3aba;
  flow_hash_info_d.hash1_sbit0_ebit9 = 0x331;
  flow_hash_info_d.hash1_sbit10_ebit10 = 1;
  flow_hash_info_d.hash2 = 0x732;
  flow_hash_info_d.hash3 = 0x733;
  flow_hash_info_d.hash4 = 0x734;
  flow_hash_info_d.hash5 = 0x735;
  flow_hash_info_d.hash6 = 0x736;
  flow_hash_info_d.hint1 = 0x121;
  flow_hash_info_d.hint2 = 0x122;
  flow_hash_info_d.hint3 = 0x123;
  flow_hash_info_d.hint4 = 0x124;
  flow_hash_info_d.hint5 = 0x125;
  flow_hash_info_d.hint6 = 0x126;
};

c1 = 1;
r1 = 0xe6a00000;
