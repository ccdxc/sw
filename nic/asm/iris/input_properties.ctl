#include "ingress.h"
#include "INGRESS_p.h"
#include "nic/hal/iris/datapath/p4/include/defines.h"

struct input_properties_k k;
struct input_properties_d d;
struct phv_               p;

d = {
  input_properties_d.vrf = 0x1;
  input_properties_d.dir = 0x1;
  input_properties_d.mdest_flow_miss_action = FLOW_MISS_ACTION_CPU;
};

c1 = 1;
