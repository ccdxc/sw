#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct flow_hash_k k;
struct flow_hash_d d;
struct phv_        p;

%%

flow_hash_info:
  seq         c2, d.flow_hash_info_d.entry_valid, 1
  bcf         [c1&c2], flow_hash_hit
  seq         c1, r1[31:21], d.flow_hash_info_d.hash1
  bcf         [c1&c2], flow_hash_hint1
  seq         c1, r1[31:21], d.flow_hash_info_d.hash2
  bcf         [c1&c2], flow_hash_hint2
  seq         c1, r1[31:21], d.flow_hash_info_d.hash3
  bcf         [c1&c2], flow_hash_hint3
  seq         c1, r1[31:21], d.flow_hash_info_d.hash4
  bcf         [c1&c2], flow_hash_hint4
  seq         c1, r1[31:21], d.flow_hash_info_d.hash5
  bcf         [c1&c2], flow_hash_hint5
  seq         c1, r1[31:21], d.flow_hash_info_d.hash6
  bcf         [c1&c2], flow_hash_hint6
  seq         c1, d.flow_hash_info_d.more_hashs, 1
  bcf         [c1&c2], flow_hash_more_hashs
  phvwr       p.control_metadata_flow_miss, 1
  phvwr       p.control_metadata_flow_miss_ingress, 1
  phvwr.e     p.flow_info_metadata_flow_index, 0
  nop

flow_hash_hit:
  phvwr       p.flow_info_metadata_flow_index[19:14], d.flow_hash_info_d.flow_index_sbit0_ebit5
  phvwr.e     p.flow_info_metadata_flow_index[13:0], d.flow_hash_info_d.flow_index_sbit6_ebit19
  phvwr       p.rewrite_metadata_entropy_hash, r1

flow_hash_hint1:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint1

flow_hash_hint2:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint2

flow_hash_hint3:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint3

flow_hash_hint4:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint4

flow_hash_hint5:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint5

flow_hash_hint6:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.hint6

flow_hash_more_hashs:
  b           flow_hash_recirc
  add         r2, r0, d.flow_hash_info_d.more_hints

flow_hash_recirc:
  phvwr       p.recirc_header_valid, 1
  phvwr       p.recirc_header_reason, RECIRC_FLOW_HASH_OVERFLOW
  phvwr       p.recirc_header_overflow_entry_index, r2
  phvwr       p.recirc_header_overflow_hash, r1
  phvwr.e     p.capri_intrinsic_tm_oport, TM_PORT_INGRESS
  phvwr       p.capri_intrinsic_tm_oq, TM_P4_IG_RECIRC_QUEUE
