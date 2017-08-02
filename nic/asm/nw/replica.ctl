#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct replica_k k;
struct phv_      p;

k = {
  tm_replication_data_lif_sbit0_ebit4 = 0;
  tm_replication_data_lif_sbit5_ebit10 = 0x2a;
  tm_replication_data_qtype = 3;
  tm_replication_data_tunnel_rewrite_index_sbit0_ebit1 = 0;
  tm_replication_data_tunnel_rewrite_index_sbit2_ebit9 = 0;
  tm_replication_data_qid_or_vnid = 0xdabada;
  tm_replication_data_rewrite_index = 0xada;
};

p = {
  tm_replication_data_valid = TRUE;
};
