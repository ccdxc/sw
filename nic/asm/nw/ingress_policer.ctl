#include "ingress.h"
#include "INGRESS_p.h"
#include "../../p4/nw/include/defines.h"

struct ingress_policer_d d;

d = {
  execute_ingress_policer_d.entry_valid = 1;
  execute_ingress_policer_d.tbkt = 0x8000000000;
};
