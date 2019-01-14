#include "ingress.h"
#include "INGRESS_p.h"
#include "apollo.h"

struct flow_k   k;
struct flow_d   d;
struct phv_     p;

d = {
  flow_hash_d.entry_valid = 1;
  flow_hash_d.flow_index = 0x3aba;
  flow_hash_d.hash1 = 0x31;
  flow_hash_d.hash2 = 0x32;
  flow_hash_d.hash3 = 0x33;
  flow_hash_d.hash4 = 0x34;
  flow_hash_d.hash5 = 0x35;
  flow_hash_d.hint1 = 0x121;
  flow_hash_d.hint2 = 0x122;
  flow_hash_d.hint3 = 0x123;
  flow_hash_d.hint4 = 0x124;
  flow_hash_d.hint5 = 0x125;
  flow_hash_d.more_hashes = 1;
  flow_hash_d.more_hints = 0x126;
  flow_hash_d.ohash_entry = 1;
};

c1 = 0;
r1 = 0x37600000;
