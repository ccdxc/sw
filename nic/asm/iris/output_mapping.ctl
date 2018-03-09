#include "egress.h"
#include "EGRESS_p.h"
#include "../../p4/iris/include/defines.h"

struct output_mapping_k k;
struct output_mapping_d d;
struct phv_             p;

d = {
  u.set_tm_oport_d.nports = 1;
  u.set_tm_oport_d.egress_port1 = TM_PORT_DMA;
  u.set_tm_oport_d.egress_port2 = TM_PORT_UPLINK_0;
  u.set_tm_oport_d.egress_port3 = TM_PORT_UPLINK_1;
  u.set_tm_oport_d.egress_port4 = TM_PORT_UPLINK_2;
  u.set_tm_oport_d.egress_port5 = TM_PORT_UPLINK_3;
  u.set_tm_oport_d.egress_port6 = TM_PORT_UPLINK_4;
  u.set_tm_oport_d.egress_port7 = TM_PORT_UPLINK_5;
  u.set_tm_oport_d.egress_port8 = TM_PORT_UPLINK_6;
};

k = {
  rewrite_metadata_entropy_hash = 0xabcd;
};
