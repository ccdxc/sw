#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct flow_hash_overflow_k k;
struct flow_hash_overflow_d d;
struct phv_                 p;

d = {
  flow_hash_overflow_d.entry_valid = 1;
  flow_hash_overflow_d.flow_index = 0xbac;
  flow_hash_overflow_d.hash1 = 0x731;
  flow_hash_overflow_d.hash2 = 0x732;
  flow_hash_overflow_d.hash3 = 0x733;
  flow_hash_overflow_d.hash4 = 0x734;
  flow_hash_overflow_d.hash5 = 0x735;
  flow_hash_overflow_d.hash6 = 0x736;
  flow_hash_overflow_d.hint1 = 0x121;
  flow_hash_overflow_d.hint2 = 0x122;
  flow_hash_overflow_d.hint3 = 0x123;
  flow_hash_overflow_d.hint4 = 0x124;
  flow_hash_overflow_d.hint5 = 0x125;
  flow_hash_overflow_d.hint6 = 0x126;
};

p = {
  recirc_header_valid = 1;
};

k = {
  recirc_header_overflow_hash = 0xe6a04545;
};

c1 = 1;
