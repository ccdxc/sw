#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct input_properties_k k;
struct input_properties_d d;
struct phv_               p;

d = {
  u.input_properties_d.vrf = 0x1;
  u.input_properties_d.dir = 0x1;
  u.input_properties_d.flow_miss_action = FLOW_MISS_ACTION_CPU;
};

k = {
  capri_intrinsic_tm_iport = TM_PORT_UPLINK_0;
  capri_p4_intrinsic_frame_size = 1900;
};
